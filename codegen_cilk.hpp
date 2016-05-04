#pragma once


#include "codegen.hpp"


class CodeGenCilk : public CodeGen {
public:
    CodeGenCilk ( 
	      std::stringstream& _output, 
	      CloogOptions* _options,  
	      std::vector<std::string>& _statement_texts,
	      std::map<std::string,std::string>& _call_texts
    ) :
      CodeGen( 
	      _output, 
	      _options,  
	      _statement_texts,
	      _call_texts
	  )
    {
    }
    virtual ~CodeGenCilk () {}

protected:

    virtual void pprint_for_loop_name( );
    
};
