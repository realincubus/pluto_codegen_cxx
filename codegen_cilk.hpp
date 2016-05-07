#pragma once


#include "codegen.hpp"


class CodeGenCilk : public CodeGen {
public:
    CodeGenCilk ( 
	      std::stringstream& _output, 
	      CloogOptions* _options,  
	      std::vector<std::string>& _statement_texts,
	      std::map<std::string,std::string>& _call_texts,
	      std::set<std::string>& _header_includes
    ) :
      CodeGen( 
	      _output, 
	      _options,  
	      _statement_texts,
	      _call_texts,
	      _header_includes
	  )
    {
    }
    virtual ~CodeGenCilk () {}

protected:

    virtual void pprint_for_loop_name( );
    
};
