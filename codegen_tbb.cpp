
#include "codegen_tbb.hpp"
#include <iostream>
#include "assert.h"

using namespace std;

void CodeGenTbb::pprint_for_loop_name( ) {
  header_includes.insert("tbb/parallel_for.h");
  dst << "tbb::parallel_for ("; 
}

void CodeGenTbb::pprint_for(struct cloogoptions *options, int indent, struct clast_for *f)
{

    pprint_time_begin( f );
    pprint_for_loop_preamble( f, indent );
    pprint_for_loop_name();

    // print the intialization
    
    if (f->LB) {
      pprint_expr(options, f->LB);
    } 

    dst << ",";

    // print the condition
    
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
