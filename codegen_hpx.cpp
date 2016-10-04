

#include "codegen_hpx.hpp"
#include <iostream>
#include "assert.h"


using namespace std;

// TODO dont include this header if we are not in the main file
void CodeGenHpx::pprint_for_loop_name( struct clast_for *f ) {
  if ((f->parallel & CLAST_PARALLEL_OMP) || (f->parallel & CLAST_PARALLEL_VEC) ) {
    header_includes.insert("hpx/hpx_main.hpp");
    // include this header 
    header_includes.insert("hpx/parallel/algorithms/for_loop.hpp");

    dst << "hpx::parallel::v2::for_loop ("; 
  }else{
    CodeGen::pprint_for_loop_name(f);
  }
}

void CodeGenHpx::pprint_for(struct cloogoptions *options, int indent, struct clast_for *f)
{

    pprint_time_begin( f );
    pprint_for_loop_preamble( f, indent );
    pprint_for_loop_name(f);

    // print the executuion policy
    
    if (f->parallel & CLAST_PARALLEL_OMP) {
      dst << "hpx::parallel::v1::par, ";
    }
    if (f->parallel & CLAST_PARALLEL_VEC) {
      dst << "hpx::parallel::v1::datapar, ";
    }

    // print the intialization
    
    if (f->LB) {
      pprint_expr(options, f->LB);
    } 

    dst << ",";

    // print the condition
    //
    // TODO this is off by one due to the half open range that is uses by pluto
    // TODO corrected this by adding 1 to it. just a preliminary solution.
    //      merge the 1 with the expression 
    if (f->UB) { 
      pprint_expr(options, f->UB);
      dst << " + 1";
    }

    dst << ",";

    // stride is inherently 1
    
    // build the lambda 
    
    dst << "[&](int " << f->iterator  << ") {\n" ; 

    // print the  for loop body

    pprint_stmt_list(indent + INDENT_STEP, f->body);

    // close the body
    pprint_indent( indent );
    dst << "} );" << endl;

    pprint_time_end( f );

}
