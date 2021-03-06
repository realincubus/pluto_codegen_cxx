#pragma once


#include "codegen.hpp"


class CodeGenCilk : public CodeGen {
public:
    CodeGenCilk ( 
	      std::stringstream& _output, 
	      CloogOptions* _options,  
	      std::vector<std::string>& _statement_texts,
	      std::map<std::string,std::string>& _call_texts,
	      std::set<std::string>& _header_includes,
              bool _print_guards
    ) :
      CodeGen( 
	      _output, 
	      _options,  
	      _statement_texts,
	      _call_texts,
	      _header_includes,
              _print_guards
	  )
    {
    }
    virtual ~CodeGenCilk () {}

protected:

    virtual void pprint_for_loop_name(struct clast_for *f ) override;
    virtual void pprint_for_loop_preamble( struct clast_for* f, int indent ) override;
    virtual void pprint_for_loop_epilogue( struct clast_for* f, int indent ) override;
    virtual std::string replace_reduction_variables( std::string statement_texts,  pluto_codegen_cxx::StatementInformation* sinfo ) override;
    
};
