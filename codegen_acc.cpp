
#include "codegen_acc.hpp"
#include <iostream>
#include "assert.h"


using namespace std;

 
// TODO rewrite the clast part to understand openacc
//      until this is not done simply asume openmp is the same as openacc
void CodeGenAcc::pprint_for_loop_preamble( struct clast_for* f, int indent ){
    if ((f->parallel & CLAST_PARALLEL_OMP) && !(f->parallel & CLAST_PARALLEL_MPI)) {
      dst << "#pragma acc kernels ";
      dst << endl;
      pprint_indent( indent );
      return;
    }
    // TODO will be emitted if it is a inner loop 
    //      acc kernels statements can not be nested !?!?
    if ((f->parallel & CLAST_PARALLEL_VEC) && !(f->parallel & CLAST_PARALLEL_OMP) && !(f->parallel & CLAST_PARALLEL_MPI)) {
      dst << "#pragma acc kernels ";
      dst << endl;
      pprint_indent( indent );
    }

    // can be reached if the loop is a inner loop
}

void CodeGenAcc::pprint_for(struct cloogoptions *options, int indent, struct clast_for *f)
{

    pprint_time_begin( f );
    pprint_for_loop_preamble( f, indent );
    pprint_for_loop_name();

    // print the intialization
    
    if (f->LB) {
	dst << "auto " << f->iterator << "=";
        if (f->parallel & (CLAST_PARALLEL_OMP | CLAST_PARALLEL_MPI)) {
	  //dst << "lbp";
	  pprint_expr(options, f->LB);
        }else if (f->parallel & CLAST_PARALLEL_VEC){
	  dst << "lbv";
        }else{
	  pprint_expr(options, f->LB);
        }
    } 

    dst << ";";

    // print the condition
    // TODO find out how to force < instead of <=
    
    if (f->UB) { 
	if (options->language != CLOOG_LANGUAGE_FORTRAN){
	    dst << f->iterator << "<=";
	}

        if (f->parallel & (CLAST_PARALLEL_OMP | CLAST_PARALLEL_MPI)) {
	    //dst << "ubp";
	    pprint_expr(options, f->UB);
        }else if (f->parallel & CLAST_PARALLEL_VEC){
	    dst << "ubv";
        }else{
            pprint_expr(options, f->UB);
        }
    }


    // print the stride

    if (cloog_int_gt_si(f->stride, 1)) {
      dst << ";" << f->iterator << "+=";
      cloog_int_print(f->stride);
      dst << ") {" << endl;
    } else {
      dst << ";++" << f->iterator << ") {" << endl;
    }

    // print the  for loop body

    pprint_stmt_list(indent + INDENT_STEP, f->body);
    pprint_indent( indent );

    // close the body
    dst << "}" << endl;

    pprint_time_end( f );

}
