
#include "codegen_omp.hpp"
#include <iostream>
#include "assert.h"


using namespace std;
using namespace pluto_codegen_cxx;

 
void CodeGenOMP::pprint_for_loop_preamble( struct clast_for* f, int indent ){
    // if it is openmp code and not mpi 
    if ((f->parallel & CLAST_PARALLEL_OMP) && !(f->parallel & CLAST_PARALLEL_MPI)) {

	dst << "#pragma omp parallel for ";
	if ( f->reduction_vars ) {
	  // unparse the reduction variables into a set
	  auto reduction_set = StatementInformation::from_string( f->reduction_vars );
	  int once = true;
	  for( auto& reduction : reduction_set ){
	    if ( once ) {
	      once = false;
	    }else{
	      dst << ", ";
	    }
	    dst << "reduction( " << StatementInformation::enum_to_op(reduction.second) << ":" << reduction.first << ")";
	  }
	  
	}
	dst << endl;
	pprint_indent( indent );
    }
    if ((f->parallel & CLAST_PARALLEL_VEC) && !(f->parallel & CLAST_PARALLEL_OMP)
	   && !(f->parallel & CLAST_PARALLEL_MPI)) {
	if (f->LB) {
	    dst << "auto lbv=";
	    pprint_expr(options, f->LB);
	    dst << ";" << endl;
	}
	if (f->UB) {
	    pprint_indent( indent );
	    dst << "auto ubv=";
	    pprint_expr(options, f->UB);
	    dst << ";" << endl;
	}
	pprint_indent( indent );
	dst << "#pragma ivdep" << endl;
	pprint_indent( indent );
	dst << "#pragma vector always" << endl;
	pprint_indent(indent );
    }
    if (f->parallel & CLAST_PARALLEL_MPI) {
	if (f->LB) {
	    dst << "_lb_dist=";
	    pprint_expr(options, f->LB);
	    dst << ";" << endl;
	}
	if (f->UB) {
	    pprint_indent( indent );
	    dst << "_ub_dist=";
	    pprint_expr(options, f->UB);
	    dst << ";" << endl;
	}
	pprint_indent( indent );
	dst << "polyrt_loop_dist(_lb_dist, _ub_dist, nprocs, my_rank, &lbp, &ubp);" << endl;
	if (f->parallel & CLAST_PARALLEL_OMP) {
	    dst << "#pragma omp parallel for ";
	    // TODO renable this if we really have private vars
#if 0
	    if ( f->private_vars ) {
	      dst << "private(" << f->private_vars << ")";
	    }
#endif
	    if ( f->reduction_vars ) {
	      dst << "reduction(" << f->reduction_vars << ")";
	    }
	    dst << endl;
	}
	pprint_indent( indent );
    }

}

void CodeGenOMP::pprint_for(struct cloogoptions *options, int indent, struct clast_for *f)
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
