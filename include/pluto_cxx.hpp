
#include <vector>
#include <string>
#include <tuple>

struct plutoProg;
typedef plutoProg PlutoProg;

namespace pluto_cxx{
  std::vector<std::tuple<int,int,std::string>> pluto_get_dep_explanations( PlutoProg* prog );
}






