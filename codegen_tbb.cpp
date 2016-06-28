
#include "codegen_tbb.hpp"
#include <iostream>
#include "assert.h"

using namespace std;
using namespace pluto_codegen_cxx;

void CodeGenTbb::pprint_for_loop_preamble( struct clast_for* f, int indent ) {

  if ((f->parallel & CLAST_PARALLEL_OMP) && !(f->parallel & CLAST_PARALLEL_MPI)) {

      if ( f->reduction_vars ) {
	header_includes.insert("tbb/combinable.h");
	// unparse the reduction variables into a set
	auto reduction_set = StatementInformation::from_string( f->reduction_vars );
	int once = true;
	for( auto& reduction : reduction_set ){
	  if ( once ) {
	    once = false;
	  }else{
	    dst << "\n";
	    pprint_indent( indent  );
	  }
	  //dst << "reduction( " << StatementInformation::enum_to_op(reduction.second) << ":" << reduction.first << ")";
	  dst << "tbb::combinable< decltype( " << reduction.first << " ) > " << reduction.first << "_combine( " << reduction.first << " );";
	}
	
	dst << endl;
	pprint_indent( indent );
      }
  }
}

void CodeGenTbb::pprint_for_loop_epilogue( struct clast_for* f, int indent ) {

  if ((f->parallel & CLAST_PARALLEL_OMP) && !(f->parallel & CLAST_PARALLEL_MPI)) {

      if ( f->reduction_vars ) {
	header_includes.insert("tbb/combinable.h");
	// unparse the reduction variables into a set
	auto reduction_set = StatementInformation::from_string( f->reduction_vars );
	int once = true;
	for( auto& reduction : reduction_set ){
	  if ( once ) {
	    once = false;
	  }else{
	    dst << "\n";
	  }
	  //dst << "reduction( " << StatementInformation::enum_to_op(reduction.second) << ":" << reduction.first << ")";
	  auto reduction_type = ""s;
	  switch (reduction.second){
	    case StatementInformation::REDUCTION_SUM:
	      reduction_type = "std::plus<>()";
	      break;
	    case StatementInformation::REDUCTION_MUL:
	      reduction_type = "std::multiplies<>()";
	      break;
	  }
	  pprint_indent( indent );
	  dst << reduction.first << " = " << reduction.first << "_combine.combine( " << reduction_type << " );";
	}
	
	dst << endl;
	pprint_indent( indent );
      }
  }
}

void
CodeGenTbb::replace_reduction_variables( std::string& statement_text, StatementInformation* sinfo ) {
  for( auto& reduction : sinfo->reductions ){
    // TODO search for all occurences of the name reduction.first
    //      and replace them with our new combine var
    auto old_name = reduction.first;
    auto new_name = old_name + "_combine.local()";
    int pos = 0;
    while( true ) {
      pos = statement_text.find( old_name, pos );
      if ( pos == string::npos ) break;
      statement_text.replace( pos, old_name.size(), new_name );
      pos += new_name.size();
    }
  }
  
}

void CodeGenTbb::pprint_for_loop_name( ) {
  header_includes.insert("tbb/parallel_for.h");
  dst << "tbb::parallel_for ("; 
}


#if 0
template <typename T>
static void cloog_int_print( std::stringstream& dst, T i ) {
  char *s;                                                
  cloog_int_print_gmp_free_t gmp_free;                    
  s = mpz_get_str(0, 10, i);                              
  dst << s;                                               	
  mp_get_memory_functions(NULL, NULL, &gmp_free);         
  (*gmp_free)(s, strlen(s)+1);                            
}
#endif

#if 0
// if the first operation is a subtraction by one 
// dont do it
// if its something else add the + 1 for the half open range
void CodeGenTbb::pprint_term_ho(struct cloogoptions* i, struct clast_term* t ) {
    std::cerr << __PRETTY_FUNCTION__ << std::endl;
    if (t->var) {
	int group = t->var->type == clast_expr_red &&
		    ((struct clast_reduction*) t->var)->n > 1;

	if (cloog_int_is_one(t->val)){
	    ;
	}else if (cloog_int_is_neg_one(t->val)){
	    dst << "-";
	} else {
	    cloog_int_print( dst, t->val);
	    dst << "*";
	}
	if (group){
	    dst << "(";
	}
	//pprint_expr(i, t->var);
	to_half_open_range( i,  t->var );
	if (group){
	    dst << ")";
	}
    } else {
	cloog_int_print( dst, t->val );
    }
}


void CodeGenTbb::pprint_binary_ho(struct cloogoptions* i, struct clast_binary* b ) {
    std::cerr << __PRETTY_FUNCTION__ << std::endl;
    const char *s1 = NULL, *s2 = NULL, *s3 = NULL;
    int group = b->LHS->type == clast_expr_red &&
		((struct clast_reduction*) b->LHS)->n > 1;
    switch (b->type) {
    case clast_bin_fdiv:
	s1 = "floord(", s2 = ",", s3 = ")";
	break;
    case clast_bin_cdiv:
	s1 = "ceild(", s2 = ",", s3 = ")";
	break;
    case clast_bin_div:
	if (group)
	    s1 = "(", s2 = ")/", s3 = "";
	else
	    s1 = "", s2 = "/", s3 = "";
	break;
    case clast_bin_mod:
	if (group)
	    s1 = "(", s2 = ")%", s3 = "";
	else
	    s1 = "", s2 = "%", s3 = "";
	break;
    }

    dst << s1;
    pprint_expr(i, b->LHS);
    dst << s2;
    cloog_int_print( dst, b->RHS);
    dst << s3;

}

bool isMinusOne( struct clast_term* t ) {
  return cloog_int_is_neg_one(t->val);
}

void CodeGenTbb::pprint_sum_ho(struct cloogoptions *opt, struct clast_reduction *r)
{
    std::cerr << __PRETTY_FUNCTION__ << std::endl;
    int i;
    struct clast_term *t;

    // TODO find a term that is minus one 
    int found_id = -1;
    for( int i = 0 ; i < r->n ; i++ ){
      if ( isMinusOne( (struct clast_term*) r->elts[i] ) ) {
	found_id = i;
	break;
      }
    }

    if ( found_id != -1 ) {
      std::cerr << "found a minus one term at " << found_id << std::endl;
    }

    if ( found_id != 0 ) {
      assert(r->n >= 1);
      assert(r->elts[0]->type == clast_expr_term);
      t = (struct clast_term *) r->elts[0];
      pprint_term_ho(opt, t);
    }

    for (i = 1; i < r->n; ++i) {
	if ( i == found_id ) continue;
	assert(r->elts[i]->type == clast_expr_term);
	t = (struct clast_term *) r->elts[i];
	if (cloog_int_is_pos(t->val)){
	    dst << "+";
	}
	pprint_term_ho(opt, t);
    }
}

void CodeGenTbb::pprint_reduction_ho(struct cloogoptions *i, struct clast_reduction *r)
{
    std::cerr << __PRETTY_FUNCTION__ << r->type << " " << clast_red_sum << std::endl;

    switch (r->type) {
    case clast_red_sum:
	pprint_sum_ho(i, r);
	break;
    case clast_red_min:
    case clast_red_max:
	if (r->n == 1) {
	    to_half_open_range(i, r->elts[0]);
	    break;
	}
	pprint_minmax_c(i, r);
	break;
    default:
	assert(0);
    }
}

#endif 

static struct clast_binary* isBinaryOp( clast_expr* e ) {
  
  if ( e->type == clast_expr_bin ) {
    std::cerr << "is a binary op" << std::endl;
    return (struct clast_binary*) e;
  }
  if ( e->type == clast_expr_red ) {
    std::cerr << "is a red write code to handle this" << std::endl;
    auto r = (struct clast_reduction*) e;
    if ( r->type == clast_red_sum ) {
      std::cerr << "red is a sum op" << std::endl;
    }else{
      std::cerr << "red is not a sum op" << std::endl;
      if ( r->type == clast_red_min ) {
	std::cerr << "red is a min op" << std::endl;
	if ( r->n == 1 ) {
	  std::cerr << "red min has 1 parameter" << std::endl;
	  auto e = r->elts[0];
	  if ( e->type == clast_expr_bin ) {
	    std::cerr << "red min e 1 is a binary op" << std::endl;
	    return (struct clast_binary*)e;
	  }
	  if ( e->type == clast_expr_red ) {
	    std::cerr << "red min e 1 is a reduction" << std::endl;
	    auto r = (struct clast_reduction*) e;
	    if ( r->type == clast_red_sum ) {
	      std::cerr << "red of red is sum" << std::endl;
	      // this is a sum operation  that means its not a tree of binary operators "+" 
	      // its a list of elements with their corresponding signs
	      // TODO i need to find a single minus 1 op in this list and replace it
	      
	    }else{
	      std::cerr << "red of red is NOT sum" << std::endl;
	    }

	  }else{
	    std::cerr << "red min e 1 is not a reduction" << std::endl;
	  }

	}else{
	  std::cerr << "red min has more than 1 parameter" << std::endl;
	}
      }else{
	std::cerr << "red is not a min op" << std::endl;
      }
    }
  }

  std::cerr << "is not a binary op" << std::endl;

  return nullptr;
}

static bool isMinusOne( struct clast_expr* e ) {
  
  return false;
}


static bool isMinusOne( cloog_int_t i ) {
  if ( cloog_int_is_neg_one(i) ){
    return true;
  }
  return false;
}

#if 0
void CodeGenTbb::to_half_open_range( struct cloogoptions* i, struct clast_expr* e ) {
#if 1
    if (!e)
	return;

    std::cerr << __PRETTY_FUNCTION__ << std::endl;

    switch( e->type ) {
    case clast_expr_name:{
	pprint_name( (struct clast_name*) e);
	break;
    }
    case clast_expr_term :{
	pprint_term_ho(i,  (struct clast_term*) e);
	return;
    }
    case clast_expr_bin: {
	pprint_binary_ho(i, (struct clast_binary*) e);
	return;
    }
    case clast_expr_red:{
	pprint_reduction_ho(i,  (struct clast_reduction*) e);
	return;
    }
    default:
	std::cerr << "case not handled " << e->type << std::endl;
    }
#endif
#if 0
    // check that this is a binary op 
    if ( auto bin_op = isBinaryOp( e ) ) {
      // TODO check that lhs or rhs is -1 
      if ( isMinusOne( bin_op->LHS ) ) {
	cloog_int_print( dst, bin_op->RHS );
	return;
      }

      if ( isMinusOne( bin_op->RHS ) ) {
	pprint_expr( i, bin_op->LHS );
	return;
      }
    }

    pprint_expr( i, e );
    dst << " + 1" ;
#endif
}
#endif

void CodeGenTbb::pprint_for(struct cloogoptions *options, int indent, struct clast_for *f)
{

    pprint_time_begin( f );
    pprint_for_loop_preamble( f, indent + 4 );
    pprint_for_loop_name();

    // print the intialization
    
    // TODO left and right side have to have the 
    // same type so instanciation of the template works
    // i can add a cast to a very long int type on both sides
    // but this is rather ugly
    // # need to get the types of both sides 
    // # determin largest of both
    // # cast the other one to the larger type
    if (f->LB) {
      //dst << "(int)";
      pprint_expr(options, f->LB);
    } 

    dst << ",";

    // print the condition
    
    // TODO this is off by one due to the half open range that is uses by pluto
    // TODO corrected this by adding 1 to it. just a preliminary solution.
    //      merge the 1 with the expression 
    if (f->UB) { 
      //dst << "(int)";
      pprint_expr(options, f->UB);
      //to_half_open_range( options, f->UB );
      dst << " + 1";
    }

    dst << ",";

    // stride is inherently 1 since if forbid pet to allow something other than 1 
    
    // build the lambda 
    
    dst << "[&](int " << f->iterator  << ") {\n" ; 

    // print the  for loop body

    pprint_stmt_list(indent + INDENT_STEP, f->body);

    // close the body
    pprint_indent( indent );
    dst << "} );" << endl;

    pprint_for_loop_epilogue( f, indent + 4  );

    pprint_time_end( f );

}
