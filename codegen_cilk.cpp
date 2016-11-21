
#include "codegen_cilk.hpp"

#include <iostream>
#include "assert.h"

using namespace std;
using namespace pluto_codegen_cxx;

void CodeGenCilk::pprint_for_loop_name( struct clast_for *f) {
  if ((f->parallel & CLAST_PARALLEL_OMP) && !(f->parallel & CLAST_PARALLEL_MPI)) {
    header_includes.insert("cilk/cilk.h");
    dst << "cilk_for ("; 
  }else{
    dst << "for ("; 
  }
}

void CodeGenCilk::pprint_for_loop_preamble( struct clast_for* f, int indent ) {

  if ((f->parallel & CLAST_PARALLEL_OMP) && !(f->parallel & CLAST_PARALLEL_MPI)) {

      if ( f->reduction_vars ) {
	// unparse the reduction variables into a set
	auto reduction_set = StatementInformation::from_string( f->reduction_vars );
	int once = true;
	for( auto& reduction : reduction_set ){
	  if ( once ) {
	    once = false;
	  }else{
	    dst << "\n";
	    pprint_indent( indent );
	  }
	  switch( reduction.second ) {
	    case StatementInformation::REDUCTION_SUM :{
		header_includes.insert("cilk/reducer_opadd.h");
		dst << "cilk::reducer<cilk::op_add< decltype( " << reduction.first << " ) >> " << reduction.first << "_reducer( " << reduction.first << " );";
	        break;
	    }
	    case StatementInformation::REDUCTION_MUL :{
		header_includes.insert("cilk/reducer_opmul.h");
		dst << "cilk::reducer<cilk::op_mul< decltype( " << reduction.first << " ) >> " << reduction.first << "_reducer( " << reduction.first << " );";
		break;
	    }
	  }
	  //dst << "reduction( " << StatementInformation::enum_to_op(reduction.second) << ":" << reduction.first << ")";
	  // TODO header depends on the reduction type
	}
	
	dst << endl;
	pprint_indent( indent );
      }
  }
}

void CodeGenCilk::pprint_for_loop_epilogue( struct clast_for* f, int indent ) {

  if ((f->parallel & CLAST_PARALLEL_OMP) && !(f->parallel & CLAST_PARALLEL_MPI)) {

      if ( f->reduction_vars ) {
	// unparse the reduction variables into a set
	auto reduction_set = StatementInformation::from_string( f->reduction_vars );
	int once = true;
	for( auto& reduction : reduction_set ){
	  if ( once ) {
	    once = false;
	  }else{
	    dst << "\n";
	  }
	  pprint_indent( indent );
	  dst << reduction.first << " = " << reduction.first << "_reducer.get_value();";
	}
	dst << endl;
	pprint_indent( indent );
      }
  }
}

void
CodeGenCilk::replace_reduction_variables( std::string& statement_text, StatementInformation* sinfo ) {
  for( auto& reduction : sinfo->reductions ){
    // TODO search for all occurences of the name reduction.first
    //      and replace them with our new combine var
    auto old_name = reduction.first;
    auto new_name = "*"s + old_name + "_reducer";
    int pos = 0;
    while( true ) {
      pos = statement_text.find( old_name, pos );
      if ( pos == string::npos ) break;
      statement_text.replace( pos, old_name.size(), new_name );
      pos += new_name.size();
    }
  }
  
}


