#pragma once


#include "codegen.hpp"


class CodeGenTbb : public CodeGen {
public:
    CodeGenTbb ( 
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
    virtual ~CodeGenTbb () {}

protected:

    virtual void pprint_for_loop_name( struct clast_for *f) override;
    virtual void pprint_for(struct cloogoptions *options, int indent, struct clast_for *f);
    virtual void pprint_for_loop_preamble( struct clast_for* f, int indent );
    virtual void pprint_for_loop_epilogue( struct clast_for* f, int indent );
    virtual std::string replace_reduction_variables( std::string statement_texts,  pluto_codegen_cxx::StatementInformation* sinfo ) override;

    void pprint_common_type_cast( struct clast_for* f );

#if 0
    void to_half_open_range( struct cloogoptions* o, struct clast_expr* e );
    void pprint_term_ho(struct cloogoptions* i, struct clast_term* t );
    void pprint_binary_ho(struct cloogoptions* i, struct clast_binary* b );
    void pprint_reduction_ho(struct cloogoptions *i, struct clast_reduction *r);
    void pprint_sum_ho(struct cloogoptions *opt, struct clast_reduction *r);
#endif
};
