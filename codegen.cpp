
#include "codegen.hpp"
#include <iostream>
#include "assert.h"

using namespace std;

void CodeGen::pprint_for_loop_name() {
    dst << "for (";
}

// TODO rewrite to use std chrono
void CodeGen::pprint_time_end( struct clast_for* f ) {
  if (f->time_var_name) {
      if ( f->time_var_name ) {
	dst << "IF_TIME(" << f->time_var_name << " += cloog_util_rtclock() - " << f->time_var_name<< "_start);" << endl;
      }
  }
}

// TODO rewrite to use std chrono 
void CodeGen::pprint_time_begin( struct clast_for* f ) {
  if (f->time_var_name) {
      if ( f->time_var_name ) {
	dst << "IF_TIME(" << f->time_var_name << " _start = cloog_util_rtclock());" << endl;
      }
  }
}

void CodeGen::pprint_equation(struct cloogoptions *i, struct clast_equation *eq)
{
    pprint_expr(i, eq->LHS);
    if (eq->sign == 0){
      dst << " == ";
    }else if (eq->sign > 0){
      dst << " >= ";
    }else{
      dst << " <= ";
    }
    pprint_expr(i, eq->RHS);
}

void CodeGen::pprint_guard(struct cloogoptions *options, int indent, struct clast_guard *g)
{
    dst << "if ";

    if (g->n > 1){
      dst << "(";
    }

    for (int k = 0; k < g->n; ++k) {
	if (k > 0) {
	  dst << " && ";
	}
	dst << "(";
	pprint_equation(options, &g->eq[k]);
	dst << ")";
    }
    if (g->n > 1){
      dst << ")";
    }
    dst << "{" << endl;

    pprint_stmt_list(indent + INDENT_STEP, g->then);
    pprint_indent( indent );

    dst << "}";
}


// TODO remove output
void CodeGen::replace_marker_with( int id , std::string& text, std::string replacement ){
  std::cout << "text: " << text << std::endl;

  std::string placeholder = "..."s + to_string(id) + "..."s;

  std::cout << "placeholder: " << placeholder << std::endl;
  std::cout << "replacement: " << replacement << std::endl;
  auto pos = text.find(placeholder);
  while ( pos != string::npos ){
    text = text.replace(pos,placeholder.length(), replacement ); 
    pos = text.find(placeholder);
  }
  std::cout << "text: " << text << std::endl;
} 

// TODO check for deprication !
/* pprint_parentheses_are_safer function:
 * this function returns 1 if it decides that it would be safer to put
 * parentheses around the clast_assignment when it is used as a macro
 * parameter, 0 otherwise.
 * \param[in] s Pointer to the clast_assignment to check.
 * \return 1 if we should print parentheses around s, 0 otherwise.
 */
int CodeGen::pprint_parentheses_are_safer(struct clast_assignment * s) {
  /* Expressions of the form X = Y should not be used in macros, so we
   * consider readability first for them and avoid parentheses.
   * Also, expressions having only one term can live without parentheses.
   */
  if ((s->LHS) ||
      (s->RHS->type == clast_expr_term) ||
      ((s->RHS->type == clast_expr_red) &&
       (((struct clast_reduction *)(s->RHS))->n == 1) &&
       (((struct clast_reduction *)(s->RHS))->elts[0]->type ==
        clast_expr_term)))
    return 0;

  return 1;
}

void CodeGen::pprint_user_stmt(struct cloogoptions *options, struct clast_user_stmt *u)
{
    int parenthesis_to_close = 0;
    struct clast_stmt *t;

    // TODO is this still needed? osl is never used with this kind of code setup
    //if (pprint_osl_body(options, u))
    //  return;
    
    if (u->statement->name){
	dst << u->statement->name;
    }else{
	
	int ctr = 0;
	for (t = u->substitutions; t; t = t->next) {
	  stringstream substitution;

	  CodeGen codegen( substitution, options, statement_texts, call_texts );

	  assert(CLAST_STMT_IS_A(t, stmt_ass));
	  if (codegen.pprint_parentheses_are_safer((struct clast_assignment *)t)) {
	    substitution << "(";
	    parenthesis_to_close = 1;
	  }
	  codegen.pprint_assignment(options, substitution, (struct clast_assignment *)t);
	  if (t->next) {
	      if (parenthesis_to_close) {
		substitution << ")";
		parenthesis_to_close = 0;
	      }
	  }

	  // at this point the substitution is generated
	  replace_marker_with( ctr++, statement_texts[u->statement->number-1], substitution.str() );

	}
	dst << statement_texts[u->statement->number-1];
    }
}

// TODO make it write std if asked to 
void CodeGen::pprint_minmax_c(struct cloogoptions *info, struct clast_reduction *r)
{
    for (int i = 1; i < r->n; ++i){
	if ( r->type == clast_red_max ) {
	  dst << "max(";
	}else{
	  dst << "min(";
	}
    }
    if (r->n > 0){
	pprint_expr(info, r->elts[0]);
    }

    for (int i = 1; i < r->n; ++i) {
	dst << ",";
	pprint_expr(info, r->elts[i]);
	dst << ")";
    }
}



void CodeGen::pprint_sum(struct cloogoptions *opt, struct clast_reduction *r)
{
    int i;
    struct clast_term *t;

    assert(r->n >= 1);
    assert(r->elts[0]->type == clast_expr_term);
    t = (struct clast_term *) r->elts[0];
    pprint_term(opt, t);

    for (i = 1; i < r->n; ++i) {
	assert(r->elts[i]->type == clast_expr_term);
	t = (struct clast_term *) r->elts[i];
	if (cloog_int_is_pos(t->val)){
	    dst << "+";
	}
	pprint_term(opt, t);
    }
}

template <typename T>
void CodeGen::cloog_int_print( T i ) {
  char *s;                                                
  cloog_int_print_gmp_free_t gmp_free;                    
  s = mpz_get_str(0, 10, i);                              
  dst << s;                                               	
  mp_get_memory_functions(NULL, NULL, &gmp_free);         
  (*gmp_free)(s, strlen(s)+1);                            
}

// TODO think about std::min max and check for adding the needed headers 
void CodeGen::pprint_binary(struct cloogoptions *i, struct clast_binary *b)
{
    const char *s1 = NULL, *s2 = NULL, *s3 = NULL;
    int group = b->LHS->type == clast_expr_red &&
		((struct clast_reduction*) b->LHS)->n > 1;
    switch (b->type) {
    case clast_bin_fdiv:
	s1 = "floord(", s2 = ",", s3 = ")";
	break;
    case clast_bin_cdiv:
	s1 = "ceild(", s2 = ",", s3 = ")";
	break;
    case clast_bin_div:
	if (group)
	    s1 = "(", s2 = ")/", s3 = "";
	else
	    s1 = "", s2 = "/", s3 = "";
	break;
    case clast_bin_mod:
	if (group)
	    s1 = "(", s2 = ")%", s3 = "";
	else
	    s1 = "", s2 = "%", s3 = "";
	break;
    }

    dst << s1;
    pprint_expr(i, b->LHS);
    dst << s2;
    cloog_int_print( b->RHS);
    dst << s3;
}


void CodeGen::pprint_reduction(struct cloogoptions *i, struct clast_reduction *r)
{
    switch (r->type) {
    case clast_red_sum:
	pprint_sum(i, r);
	break;
    case clast_red_min:
    case clast_red_max:
	if (r->n == 1) {
	    pprint_expr(i, r->elts[0]);
	    break;
	}
	pprint_minmax_c(i, r);
	break;
    default:
	assert(0);
    }
}

void CodeGen::pprint_name(struct clast_name *n)
{
  std::cerr << "printing name " << n->name << std::endl;
  auto id = call_texts.find( n->name );
  if ( id != call_texts.end() ) { 
    dst << id->second;
  }else{
    dst << n->name;
  }
}

/**
 * This function returns a string containing the printing of a value (possibly
 * an iterator or a parameter with its coefficient or a constant).
 * - val is the coefficient or constant value,
 * - name is a string containing the name of the iterator or of the parameter,
 */
void CodeGen::pprint_term(struct cloogoptions *i, struct clast_term *t)
{
    if (t->var) {
	int group = t->var->type == clast_expr_red &&
		    ((struct clast_reduction*) t->var)->n > 1;

	if (cloog_int_is_one(t->val)){
	    ;
	}else if (cloog_int_is_neg_one(t->val)){
	    dst << "-";
	} else {
	    cloog_int_print( t->val);
	    dst << "*";
	}
	if (group){
	    dst << "(";
	}
	pprint_expr(i, t->var);
	if (group){
	    dst << ")";
	}
    } else {
	cloog_int_print( t->val);
    }
}

void CodeGen::pprint_expr(struct cloogoptions *i, struct clast_expr *e)
{
    if (!e)
	return;
    switch (e->type) {
    case clast_expr_name:
	pprint_name( (struct clast_name*) e);
	break;
    case clast_expr_term:
	pprint_term(i,  (struct clast_term*) e);
	break;
    case clast_expr_red:
	pprint_reduction(i,  (struct clast_reduction*) e);
	break;
    case clast_expr_bin:
	pprint_binary(i, (struct clast_binary*) e);
	break;
    default:
	assert(0);
    }
}

void CodeGen::pprint_assignment(struct cloogoptions *i, std::stringstream& dst, struct clast_assignment *a)
{
    if (a->LHS){
      dst << a->LHS << " = ";
    }
    pprint_expr(i,  a->RHS);
}

void CodeGen::pprint_indent( int indent ){
    for (int i = 0; i < indent; ++i){
      dst << " "; 
    }
}

void CodeGen::pprint_for( struct cloogoptions *options, int indent, struct clast_for *f ) {

    pprint_time_begin( f );
    pprint_for_loop_preamble( f, indent );
    pprint_for_loop_name();

    // print the intialization
    
    if (f->LB) {
	dst << "auto " << f->iterator << "=";
	pprint_expr(options, f->LB);
    } 

    dst << ";";

    // print the condition
    // TODO find out how to force < instead of <=
    
    if (f->UB) { 
	dst << f->iterator << "<=";
	pprint_expr(options, f->UB);
    }


    // print the stride

    if (cloog_int_gt_si(f->stride, 1)) {
      dst << ";" << f->iterator << "+=";
      cloog_int_print(f->stride);
      dst << ") {" << endl;
    } else {
      dst << ";++" << f->iterator << ") {" << endl;
    }

    // print the  for loop body

    pprint_stmt_list(indent + INDENT_STEP, f->body);
    pprint_indent( indent );

    // close the body
    dst << "}" << endl;

    pprint_time_end( f );

}

void CodeGen::pprint_for_loop_preamble( struct clast_for* f, int indent ){

}

void CodeGen::pprint_stmt_list(
    int indent,
    struct clast_stmt *s
)
{
    for ( ; s; s = s->next) {
	if (CLAST_STMT_IS_A(s, stmt_root))
	    continue;

	pprint_indent( indent );
	if (CLAST_STMT_IS_A(s, stmt_ass)) {
	    pprint_assignment(options, dst, (struct clast_assignment *) s);
	    dst << ";" << endl;
	} else if (CLAST_STMT_IS_A(s, stmt_user)) {
	    pprint_user_stmt(options, (struct clast_user_stmt *) s);
	} else if (CLAST_STMT_IS_A(s, stmt_for)) {
	    pprint_for(options, indent, (struct clast_for *) s);
	} else if (CLAST_STMT_IS_A(s, stmt_guard)) {
	    pprint_guard(options, indent, (struct clast_guard *) s);
	} else if (CLAST_STMT_IS_A(s, stmt_block)) {
	    dst << "{" << endl;
	    pprint_stmt_list(indent + INDENT_STEP, ((struct clast_block *)s)->body);
	    pprint_indent( indent );
	    dst << "}" << endl;
	} else {
	    assert(0);
	}
    }
}

void CodeGen::pprint(
    struct clast_stmt *root,
    int indent 
)
{
  pprint_stmt_list(indent, root);
}
