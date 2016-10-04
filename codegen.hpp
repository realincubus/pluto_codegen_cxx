#pragma once
// This class is a abstract base class for CodeGeneration and will 
// be used to subclass for different code genration styles

#include <map>
#include <set>
#include <vector>
#include <string>
#include <sstream>
#include <cloog/cloog.h>
#include <cloog/clast.h>
#include "pluto_codegen_cxx.hpp"

#undef cloog_int_print

class CodeGen {
public:
    CodeGen ( std::stringstream& _output, 
	      CloogOptions* _options,  
	      std::vector<std::string>& _statement_texts,
	      std::map<std::string,std::string>& _call_texts,
	      std::set<std::string>& _header_includes
    ) : 
      dst(_output),
      options(_options),
      statement_texts(_statement_texts),
      call_texts(_call_texts),
      header_includes(_header_includes)

    {
    }
    virtual ~CodeGen () {
    
    }

    virtual void pprint(
	struct clast_stmt *root, 
	int indent 
    );


    virtual void pprint_stmt_list( int indent, struct clast_stmt *s ) ;
    virtual void pprint_indent( int indent );
    virtual void pprint_assignment(struct cloogoptions *i, std::stringstream& dst, struct clast_assignment *a);
    virtual void pprint_expr(struct cloogoptions *i, struct clast_expr *e);
    virtual void pprint_name(struct clast_name *n);
    virtual void pprint_term(struct cloogoptions *i, struct clast_term *t);
    virtual void pprint_reduction(struct cloogoptions *i, struct clast_reduction *r);
    virtual void pprint_binary(struct cloogoptions *i, struct clast_binary *b);
    //template <typename T>
    //void cloog_int_print(T i);

    virtual void pprint_sum(struct cloogoptions *opt, struct clast_reduction *r);
    virtual void pprint_minmax_c(struct cloogoptions *info, struct clast_reduction *r);
    virtual void pprint_user_stmt(struct cloogoptions *options, struct clast_user_stmt *u);
    virtual int  pprint_parentheses_are_safer(struct clast_assignment * s);
    virtual std::string replace_marker_with( int id , std::string text, std::string replacement );
    virtual void pprint_for(struct cloogoptions *options, int indent, struct clast_for *f);
    virtual void pprint_guard(struct cloogoptions *options, int indent, struct clast_guard *g);
    virtual void pprint_equation(struct cloogoptions *i, struct clast_equation *eq);
    virtual void pprint_time_begin( struct clast_for* f );
    virtual void pprint_time_end( struct clast_for* f );
    /// @brief the part before the for ( ... ) begins
    virtual void pprint_for_loop_preamble( struct clast_for* f, int indent );
    /// @brief the part after the for loop
    virtual void pprint_for_loop_epilogue( struct clast_for* f, int indent );
    /// @brief the name of the for loop ( for, cilk_for .. )
    virtual void pprint_for_loop_name( struct clast_for *f );
    virtual void pprint_if_qualified( std::string, std::string );
    virtual void replace_reduction_variables( std::string& statement_texts, pluto_codegen_cxx::StatementInformation* sinfo );
  

protected:

    std::stringstream& dst;
    CloogOptions* options = nullptr;
    std::vector<std::string>& statement_texts;
    std::map<std::string,std::string>& call_texts;
    std::set<std::string>& header_includes;

    bool qualified_names = true;
    
};

template <typename T>
inline void cloog_int_print( std::stringstream& dst,  T i ) {
  char *s;                                                
  cloog_int_print_gmp_free_t gmp_free;                    
  s = mpz_get_str(0, 10, i);                              
  dst << s;                                               	
  mp_get_memory_functions(NULL, NULL, &gmp_free);         
  (*gmp_free)(s, strlen(s)+1);                            
}
