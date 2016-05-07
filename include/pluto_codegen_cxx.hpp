
//#include "pluto.h"

struct plutoProg;
typedef plutoProg PlutoProg;

#include <sstream>
#include <vector>
#include <map>
#include <set>

namespace pluto_codegen_cxx{

  enum EMIT_CODE_TYPE{
      EMIT_ACC,
      EMIT_OPENMP,
      EMIT_HPX,
      EMIT_CILK,
      EMIT_TBB,
      EMIT_LIST_END
  };

  int pluto_multicore_codegen( std::stringstream& outfp, 
      const PlutoProg *prog, 
      std::vector<std::string>& statement_texts,
      EMIT_CODE_TYPE emit_code_type,
      bool write_cloog_file,
      std::map<std::string, std::string>& call_texts,
      std::set<std::string>& header_includes
      );


}
