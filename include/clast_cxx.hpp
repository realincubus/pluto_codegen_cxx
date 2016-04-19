
#ifndef _CLAST_CLANG_HPP
#define _CLAST_CLANG_HPP 

#include <sstream>
#include <vector>

namespace clast_cxx_omp{
  void clast_pprint(
      std::stringstream& foo, 
      struct clast_stmt *root, 
      int indent, 
      CloogOptions *options, 
      std::vector<std::string>& statement_texts 
  );
}
namespace clast_cxx_acc{
  void clast_pprint(
      std::stringstream& foo, 
      struct clast_stmt *root, 
      int indent, 
      CloogOptions *options, 
      std::vector<std::string>& statement_texts 
  );
}

#endif
