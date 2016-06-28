
#include "pluto_cxx.hpp"
#include "program.h"

namespace pluto_cxx{

  std::vector<std::tuple<int,int,std::string>> pluto_get_dep_explanations( PlutoProg* prog ){
    std::vector<std::tuple<int,int,std::string>> ret;
    if ( prog->deps_explanation ) {
      for (int i = 0; i < prog->ndeps; ++i){
        if ( prog->deps_explanation[i] ) {
	  ret.emplace_back( prog->deps[i]->src, prog->deps[i]->dest, prog->deps_explanation[i] );
	}else{
	  ret.emplace_back( 0,0,"" );
	}
      }
    }
    return ret;
  }
}
