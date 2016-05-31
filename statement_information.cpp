

#include "pluto_codegen_cxx.hpp"
#include <utility>
#include <iostream>

using namespace std;
namespace pluto_codegen_cxx{

typedef StatementInformation::reduction_set reduction_set;

StatementInformation::ReductionOperation StatementInformation::op_to_enum( std::string op ) {
 if ( op == "+" ) {
    return REDUCTION_SUM;
 } 
 if ( op == "*" ) {
    return REDUCTION_MUL;
 }
 std::cerr << "codegen: not implemented" << std::endl;
 exit(-1);
}


std::string StatementInformation::enum_to_op( StatementInformation::ReductionOperation op ) {
 if ( op == REDUCTION_SUM ) {
    return "+";
 } 
 if ( op == REDUCTION_MUL ) {
    return "*";
 }
 std::cerr << "codegen: not implemented" << std::endl;
 exit(-1);
}

reduction_set
StatementInformation::from_string( std::string rvars ) {

  reduction_set ret;

  std::cerr << "codegen: rvars is " << rvars << std::endl;

  // splice by , 
  stringstream sstr( rvars );
  std::string pair; // pair of var and operator
  while( getline ( sstr, pair , ',' ) ) {

    // TODO splice by "/" to get the var and the operator
    stringstream sstr( pair );

    std::string var;
    getline( sstr, var, '/' );
    std::string string_op;
    getline( sstr, string_op, '/' );

    std::cerr << "codegen: adding " << var << " and op " << string_op << " to set"  << std::endl;

    auto op = op_to_enum( string_op );

    ret.insert( make_pair( var, op ) );
  }
  return ret;
}

std::string 
StatementInformation::to_string( const reduction_set& reductions ){

    // reconstruct the string
    int nreductions = reductions.size();
    int ctr = 0;
    std::string rvars = "";
    std::cerr << "codegen: n reductions " << reductions.size() << std::endl;
    for( auto& reduction : reductions ){
      std::cerr << "codegen: redcution on " << reduction.first << std::endl;
      // construct a reduction operation that is expressed as var/op ( to fullfill clast interface ) 
      rvars += reduction.first + string("/") + enum_to_op( reduction.second ) ;
      if ( ctr + 1 < nreductions ){
	rvars += ",";
      }
      ctr++;
    }
    return rvars;
}

}
