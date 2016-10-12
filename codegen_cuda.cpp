
#include "codegen_cuda.hpp"
#include <iostream>
#include "assert.h"

// generate cuda+thrust code 

using namespace std;
using namespace pluto_codegen_cxx;


void CodeGenCuda::pprint_for_loop_name(struct clast_for *f) {
  if ((f->parallel & CLAST_PARALLEL_OMP) || (f->parallel & CLAST_PARALLEL_VEC) ) {
    header_includes.insert("thrust/for_each.h");
    dst << "thrust::for_each ("; 
  }else{
    CodeGen::pprint_for_loop_name( f ); 
  }
}

void CodeGenCuda::pprint_for(struct cloogoptions *options, int indent, struct clast_for *f)
{

  indent = 2;
  // need own scop for our generated lambdas
  dst << "{\n";

  indent += 2;
  // need to generate lambdas here otherwise 
  // we cuda (version 8) will not understand nested calls to these functions 

  pprint_indent( indent );
  dst << "auto lam = __device__ __host__ [=](int " << f->iterator  << ") {\n" ; 

  // print the  for loop body
  pprint_stmt_list(indent + INDENT_STEP, f->body);

  // close the body
  pprint_indent( indent );
  dst << "};" << endl;

  pprint_time_begin( f );

  pprint_indent( indent );
  pprint_for_loop_preamble( f, indent );
  pprint_for_loop_name( f );

  // print the intialization
  if (f->LB) {
      pprint_expr(options, f->LB);
  } 

  dst << ", ";

  // print the upper bound
  if (f->UB) { 
      pprint_expr(options, f->UB);
      dst << " + 1, ";
  }

  dst << "lam );\n";


  pprint_time_end( f );

  indent -= 2;
  // end the scope
  pprint_indent( indent );
  dst << "}\n";

}








