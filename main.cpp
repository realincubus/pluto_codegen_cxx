
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include <sstream>
#include <map>
#include <iostream>
#include <chrono>
#include <functional>

#include <cloog/cloog.h>

//#include "version.h"

#include "pluto/libpluto.h"

#include "program.h"
// this might not be needed anymore
extern "C"{
#include "constraints.h"
#include "math_support.h"
void pluto_mark_parallel(struct clast_stmt *root, const PlutoProg *prog, CloogOptions *options);
void pluto_mark_vector(struct clast_stmt *root, const PlutoProg *prog, CloogOptions *options);
//int pluto_is_hyperplane_scalar(const Stmt *stmt, int level);
}

#include "pluto_codegen_cxx.hpp"
#include "clast_cxx.hpp"

#include "codegen.hpp"
#include "codegen_omp.hpp"
#include "codegen_acc.hpp"
#include "codegen_cilk.hpp"
#include "codegen_tbb.hpp"
#include "codegen_hpx.hpp"
#include "codegen_cuda.hpp"

using namespace std;

namespace pluto_codegen_cxx{

static int get_first_point_loop(Stmt *stmt, const PlutoProg *prog)
{
    int i, first_point_loop;

    if (stmt->type != ORIG) {
        for (i=0; i<prog->num_hyperplanes; i++)   {
            if (!pluto_is_hyperplane_scalar(stmt, i)) {
                return i;
            }
        }
        /* No non-scalar hyperplanes */
        return 0;
    }

    for (i=stmt->last_tile_dim+1; i<stmt->trans->nrows; i++)   {
        if (stmt->hyp_types[i] == H_LOOP)  break;
    }

    if (i < prog->num_hyperplanes) {
        first_point_loop = i;
    }else{
        /* Should come here only if
         * it's a 0-d statement */
        first_point_loop = 0;
    }

    return first_point_loop;
}

struct ParentWrapper {
    clast_stmt* stmt;
    ParentWrapper* parent;
};

typedef std::function< void( clast_stmt*, ParentWrapper ) > callback;


void visit_ast_node( clast_stmt* node, ParentWrapper parent, const clast_stmt_op& type, callback c ) {

  // recursion end
  if ( node == nullptr ) return;

  if (CLAST_STMT_IS_A(node, stmt_root)) {
    struct clast_root *root = (struct clast_root *) node;
    visit_ast_node( (root->stmt).next, parent, type, c );
  }
  if (CLAST_STMT_IS_A(node, stmt_guard)) {
    struct clast_guard *guard = (struct clast_guard *) node;
    visit_ast_node( guard->then, { node, &parent }, type, c );
    visit_ast_node( (guard->stmt).next, parent, type, c  );
  }

  if (CLAST_STMT_IS_A(node, stmt_user)) {
    struct clast_user_stmt *user_stmt = (struct clast_user_stmt *) node;
    visit_ast_node( (user_stmt->stmt).next, parent, type, c );
    if ( &type == &stmt_user ){
      c( node, parent );
    }
  }

  if (CLAST_STMT_IS_A(node, stmt_for)) {
    struct clast_for *for_stmt = (struct clast_for *) node;
    visit_ast_node( for_stmt->body, { node, &parent} , type, c );
    visit_ast_node( (for_stmt->stmt).next, parent, type, c );
  }
}

struct StatementInfo {
    enum STATEMENT_TYPE{
      REDUCTION_SUM,
      REDUCTION_MUL,
      REDUCTION_MAX,
      REDUCTION_MIN
    };

    STATEMENT_TYPE type;

    char var[200];

};


// searches till root and marks the first for loop with the reductions specified in sinfo
void mark_reduction( ParentWrapper parent, StatementInformation* sinfo ) {

   // recursion end
  if ( parent.stmt == nullptr ) return;

  if (CLAST_STMT_IS_A(parent.stmt, stmt_root)) {
    return;
  }
  if (CLAST_STMT_IS_A(parent.stmt, stmt_guard)) {
    mark_reduction( *parent.parent, sinfo );
    return;
  }

  if (CLAST_STMT_IS_A(parent.stmt, stmt_user)) {
    mark_reduction( *parent.parent, sinfo );
    return;
  }

  if (CLAST_STMT_IS_A(parent.stmt, stmt_for)) {
    struct clast_for *for_stmt = (struct clast_for *) parent.stmt;
    std::cerr << "codegen: found enclosing loop" << std::endl;

    auto& reductions = sinfo->reductions;

    if ( for_stmt->reduction_vars ) {
      auto present_reductions = StatementInformation::from_string( for_stmt->reduction_vars );

      // fuse the present reductions with the new ones
      for( auto& element : present_reductions ){
	reductions.insert( element );
      }

    }


    

    if ( reductions.size() == 0 ) return;

    auto new_reductions = StatementInformation::to_string( reductions );
    
    // store it back as a c string
    for_stmt->reduction_vars = strdup( new_reductions.c_str() );

    return;
  } 
}

void update_clast_reduction_information( clast_stmt* root, const PlutoProg* prog )
{
  // TODO visit every user_statement
  //      seams i need to use the clast filter function
  char* iter = nullptr;
  int* stmtids = nullptr;
  int my_nstmts = 0;
  ClastFilter filter = {iter, stmtids, my_nstmts, subset}; 

  clast_for** loops;
  int nloops;
  int* stmts;
  int nstmts;
  clast_filter( root, filter, &loops, &nloops, &stmts, &nstmts );

  std::cerr << "codegen: got " << nloops << " " << nstmts << std::endl;

  // TODO dont i have to tag all for loops with the reduction clause ? 
  // get the statement in this ast and move the reduction to the enclosing for loop 
  visit_ast_node( root, { nullptr, nullptr }, stmt_user, 
      [&](clast_stmt* user, ParentWrapper parent){
	std::cerr << "got the user_stmt from the ast" << std::endl;
	clast_user_stmt* ustmt = (clast_user_stmt*)user;
	auto stmt = ustmt->statement;
	cloog_statement_print( stderr, stmt );
	std::cerr << "codegen: statement properties " << stmt->state << " " << stmt->number << " " << stmt->usr << std::endl;

	int id = stmt->number - 1;
	assert( id >= 0 && id < prog->nstmts && "out of bounds" );
	
	// query info from the pluto program
	auto pstmt = prog->stmts[id]; 

	if ( pstmt->user ) {
	  stmt->usr = pstmt->user;
	  std::cerr << "codegen: adding statement information" << std::endl;
	  auto sinfo = (StatementInformation*)stmt->usr;
	  std::cerr << "codegen: marking reduction" << std::endl;
	  mark_reduction( parent, sinfo );
	}
      } 
  );

}

/* Call cloog and generate code for the transformed program
 *
 * cloogf, cloogl: set to -1 if you want the function to decide
 *
 * --cloogf, --cloogl overrides everything; next cloogf, cloogl if != -1,
 *  then the function takes care of the rest
 */
int pluto_gen_cloog_code_cxx(const PlutoProg *prog, int cloogf, int cloogl,
        stringstream& outfp, FILE* cloogfp, vector<std::string>& statement_texts, EMIT_CODE_TYPE emit_code_type,
	std::map<std::string,std::string>& call_texts,
	std::set<std::string>& header_includes
    )
{
    CloogInput *input ;
    CloogOptions *cloogOptions ;
    CloogState *state;
    int i;

    struct clast_stmt *root;

    Stmt **stmts = prog->stmts;
    int nstmts = prog->nstmts;

    state = cloog_state_malloc();
    cloogOptions = cloog_options_malloc(state);

    cloogOptions->fs = (int*)malloc (nstmts*sizeof(int));
    cloogOptions->ls = (int*)malloc(nstmts*sizeof(int));
    cloogOptions->fs_ls_size = nstmts;

    for (i=0; i<nstmts; i++) {
        cloogOptions->fs[i] = -1;
        cloogOptions->ls[i] = -1;
    }

    cloogOptions->name = "CLooG file produced by PLUTO";
    cloogOptions->compilable = 0;
    cloogOptions->esp = 1;
    cloogOptions->strides = 1;
    cloogOptions->quiet = options->silent;

    /* Generates better code in general */
    cloogOptions->backtrack = options->cloogbacktrack;

    if (options->cloogf >= 1 && options->cloogl >= 1) {
        cloogOptions->f = options->cloogf;
        cloogOptions->l = options->cloogl;
    }else{
        if (cloogf >= 1 && cloogl >= 1) {
            cloogOptions->f = cloogf;
            cloogOptions->l = cloogl;
        }else if (options->tile)   {
            for (i=0; i<nstmts; i++) {
                cloogOptions->fs[i] = get_first_point_loop(stmts[i], prog)+1;
                cloogOptions->ls[i] = prog->num_hyperplanes;
            }
        }else{
            /* Default */
            cloogOptions->f = 1;
            /* last level to optimize: number of hyperplanes;
             * since Pluto provides full-ranked transformations */
            cloogOptions->l = prog->num_hyperplanes;
        }
    }

    if (!options->silent)   {
        if (nstmts >= 1 && cloogOptions->fs[0] >= 1) {
            printf("[pluto] using statement-wise -fs/-ls options: ");
            for (i=0; i<nstmts; i++) {
                printf("S%d(%d,%d), ", i+1, cloogOptions->fs[i], 
                        cloogOptions->ls[i]);
            }
            printf("\n");
        }else{
            printf("[pluto] using Cloog -f/-l options: %d %d\n", 
                    cloogOptions->f, cloogOptions->l);
        }
    }

    if (options->cloogsh)
        cloogOptions->sh = 1;

    cloogOptions->name = "PLUTO-produced CLooG file";

    {
      //outfp << "/* Start of CLooG code */" << endl;
      /* Get the code from CLooG */
      printf("[pluto] cloog_input_read\n");
      input = cloog_input_read(cloogfp, cloogOptions) ;
      if ( !input ) {
	std::cout << "could not extract a cloog representation int " << __PRETTY_FUNCTION__ << std::endl;
	return EXIT_FAILURE;
      }
      auto begin = std::chrono::high_resolution_clock::now();
      printf("[pluto] cloog_clast_create\n");
      root = cloog_clast_create_from_input(input, cloogOptions);

      auto end = std::chrono::high_resolution_clock::now();

      std::chrono::duration<double> diff = end-begin;
      std::cerr << "cloog input and clast create time consumption " << diff.count() << " s" << std::endl;
    }

    // update the clast with information about reductions
    update_clast_reduction_information( root, prog );

    if (options->prevector) {
	auto begin = std::chrono::high_resolution_clock::now();
        pluto_mark_vector(root, prog, cloogOptions);
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> diff = end-begin;
	std::cerr << "mark_vector time consumption " << diff.count() << " s" << std::endl;
    }
    if (options->parallel) {
	auto begin = std::chrono::high_resolution_clock::now();
        pluto_mark_parallel(root, prog, cloogOptions);
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> diff = end-begin;
	std::cerr << "mark_parallel time consumption " << diff.count() << " s" << std::endl;
    }
    auto begin = std::chrono::high_resolution_clock::now();

    switch( emit_code_type) {

      case EMIT_ACC :{
	  CodeGenAcc codegen_acc( outfp, cloogOptions, statement_texts, call_texts, header_includes );
	  codegen_acc.pprint( root, 0 );
          break;
      }
      case EMIT_OPENMP :{
	  CodeGenOMP codegen_omp( outfp, cloogOptions, statement_texts, call_texts, header_includes );
	  codegen_omp.pprint( root, 0 );
          break;
      }
      case EMIT_TBB :{
	  CodeGenTbb codegen_tbb( outfp, cloogOptions, statement_texts, call_texts, header_includes );
	  codegen_tbb.pprint( root, 0 );
          break;
      }
      case EMIT_CILK :{
	  CodeGenCilk codegen_cilk( outfp, cloogOptions, statement_texts, call_texts, header_includes );
	  codegen_cilk.pprint( root, 0 );
          break;
      }
      case EMIT_HPX :{
	  CodeGenHpx codegen_hpx( outfp, cloogOptions, statement_texts, call_texts, header_includes );
	  codegen_hpx.pprint( root, 0 );
          break;
      }
      case EMIT_CUDA :{
	  CodeGenCuda codegen_cuda( outfp, cloogOptions, statement_texts, call_texts, header_includes );
	  codegen_cuda.pprint( root, 0 );
          break;
      }
      default:
	std::cout << "not impemented" << std::endl;
	return -1;
    }

    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> diff = end-begin;
    std::cerr << "code_emission time consumption " << diff.count() << " s" << std::endl;

    cloog_clast_free(root);

    //fprintf(outfp, "/* End of CLooG code */\n");
    //outfp << "/* End of CLooG code */" << endl;

    cloog_options_free(cloogOptions);
    cloog_state_free(state);

    return 0;
}

static void gen_stmt_macro(const Stmt *stmt, stringstream& outfp)
{
    int j;

    for (j=0; j<stmt->dim; j++) { 
        if (stmt->iterators[j] == NULL) {
            printf("Iterator name not set for S%d; required \
                    for generating declarations\n", stmt->id+1);
            assert(0);
        }
    }
    outfp << "#define S"<< stmt->id+1;
    outfp << "(";
    for (j=0; j<stmt->dim; j++)  { 
        if (j!=0) {
	  outfp << ",";
	}
        outfp << stmt->iterators[j];
    }
    outfp << ")\t";

    outfp << stmt->text << endl;
}


/* Generate code for a single multicore; the ploog script will insert openmp
 * pragmas later */
int pluto_multicore_codegen( stringstream& outfp, 
    const PlutoProg *prog, 
    vector<std::string>& statement_texts,
    EMIT_CODE_TYPE emit_code_type,
    bool write_cloog_file,
    std::map<std::string, std::string>& call_texts,
    std::set<std::string>& header_includes
    )
{ 

    std::cerr << "arrived call_texts " << std::endl;
    for( auto&& element : call_texts ){
        std::cerr << element.first << " " << element.second << std::endl;
    }
    

    // cloog has to generate some file that can then be read by clast
    // to make it faster and thread save, we put this into a memory buffer 
    size_t in_memory_file_size = 2*1024*1024;
    char in_memory_file[in_memory_file_size]; // 2MB should be ok for this crutch if this becomes a problem rewrite the code to use streams
    FILE* cloogfp = fmemopen( in_memory_file, in_memory_file_size, "w" ); 
    pluto_gen_cloog_file(cloogfp, prog);
    fprintf(cloogfp, "\n");
    fclose( cloogfp );


    if ( write_cloog_file ) {
      // TODO make filename relative to file handled
      FILE* debug_cloogfp = fopen( "pluto.cloog", "w" ); 
      pluto_gen_cloog_file(debug_cloogfp, prog);
      fclose( debug_cloogfp );
    }

    cloogfp = fmemopen( in_memory_file, in_memory_file_size, "r" );


    if (options->multipar) {
      assert( 0 && "not tested" );
      //fprintf(outfp, "\tomp_set_nested(1);\n");
      outfp << "\tomp_set_nested(1);" << endl;
      //fprintf(outfp, "\tomp_set_num_threads(2);\n");
      outfp << "\tomp_set_num_threads(2);" << endl;
    }   

    std::cout << "pluto_codegen_cxx ndeps " <<  prog->ndeps << std::endl;

    return pluto_gen_cloog_code_cxx(prog, -1, -1, outfp, cloogfp, statement_texts, emit_code_type, call_texts, header_includes );
}

}
