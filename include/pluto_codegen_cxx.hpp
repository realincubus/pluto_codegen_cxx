#pragma once

//#include "pluto.h"

struct plutoProg;
typedef plutoProg PlutoProg;

#include <sstream>
#include <vector>
#include <map>
#include <set>

namespace pluto_codegen_cxx{

  struct StatementInformation {
      std::string statement_text;

      enum ReductionOperation {
	REDUCTION_MAX,
	REDUCTION_MIN,
	REDUCTION_MUL,
	REDUCTION_SUM
      };
      typedef std::set<
	std::pair<
	  std::string,ReductionOperation
	>
      > reduction_set;
      
      reduction_set reductions;

      static reduction_set	from_string( std::string );
      static std::string	to_string( const reduction_set& );
      static ReductionOperation op_to_enum( std::string op );
      static std::string	enum_to_op( StatementInformation::ReductionOperation op );

      // TODO store the original clang ast node 

  };

  enum EMIT_CODE_TYPE{
      EMIT_ACC,
      EMIT_OPENMP,
      EMIT_HPX,
      EMIT_CILK,
      EMIT_TBB,
      EMIT_CUDA,
      EMIT_LIST_END
  };

  int pluto_multicore_codegen( std::stringstream& outfp, 
      const PlutoProg *prog, 
      std::vector<std::string>& statement_texts,
      EMIT_CODE_TYPE emit_code_type,
      bool write_cloog_file,
      std::map<std::string, std::string>& call_texts,
      std::set<std::string>& header_includes,
      bool print_guards
      );


}

