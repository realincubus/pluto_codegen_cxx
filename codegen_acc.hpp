#pragma once


#include "codegen.hpp"


class CodeGenAcc : public CodeGen {
public:
    CodeGenAcc ( 
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
    virtual ~CodeGenAcc () {}

protected:

    virtual void pprint_for(struct cloogoptions *options, int indent, struct clast_for *f);
    virtual void pprint_for_loop_preamble( struct clast_for* f, int indent );
    
};