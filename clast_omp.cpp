

// should be c++ capable
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <cloog/cloog.h>
#include <cloog/clast.h>
#include <sstream>
#include <iostream>
#include <vector>

#include "clast_cxx.hpp"

#undef cloog_int_print
#define cloog_int_print(out,i)                                          \
        do {                                                            \
                char *s;                                                \
                cloog_int_print_gmp_free_t gmp_free;                    \
                s = mpz_get_str(0, 10, i);                              \
		out << s;                                               \	
                mp_get_memory_functions(NULL, NULL, &gmp_free);         \
                (*gmp_free)(s, strlen(s)+1);                            \
        } while (0)


using namespace std;

namespace clast_cxx_omp{


static void pprint_name(std::stringstream &dst, struct clast_name *n);
static void pprint_term(struct cloogoptions *i, std::stringstream &dst, struct clast_term *t);
static void pprint_sum(struct cloogoptions *opt,
			std::stringstream &dst, struct clast_reduction *r);
static void pprint_binary(struct cloogoptions *i,
			std::stringstream &dst, struct clast_binary *b);
static void pprint_minmax_f(struct cloogoptions *info,
			std::stringstream &dst, struct clast_reduction *r);
static void pprint_minmax_c(struct cloogoptions *info,
			std::stringstream *dst, struct clast_reduction *r);
static void pprint_reduction(struct cloogoptions *i,
			std::stringstream &dst, struct clast_reduction *r);
static void pprint_expr(struct cloogoptions *i, stringstream& dst, struct clast_expr *e);
static void pprint_equation(struct cloogoptions *i,
			std::stringstream &dst, struct clast_equation *eq);
static void pprint_assignment(struct cloogoptions *i, stringstream& dst, 
			struct clast_assignment *a);
static void pprint_user_stmt(struct cloogoptions *options, std::stringstream &dst,
		       struct clast_user_stmt *u);
static void pprint_guard(struct cloogoptions *options, std::stringstream &dst, int indent,
		   struct clast_guard *g);
static void pprint_for(struct cloogoptions *options, std::stringstream &dst, int indent,
		 struct clast_for *f);
static void pprint_stmt_list(struct cloogoptions *options, stringstream& dst, int indent,
		       struct clast_stmt *s);

vector<std::string>* global_statement_texts;
std::map<std::string,std::string>* global_call_texts;


void pprint_name(std::stringstream& dst, struct clast_name *n)
{
  std::cerr << "printing name " << n->name << std::endl;
  auto id = global_call_texts->find( n->name );
  if ( id != global_call_texts->end() ) { 
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
void pprint_term(struct cloogoptions *i, std::stringstream &dst, struct clast_term *t)
{
    if (t->var) {
	int group = t->var->type == clast_expr_red &&
		    ((struct clast_reduction*) t->var)->n > 1;
	if (cloog_int_is_one(t->val))
	    ;
	else if (cloog_int_is_neg_one(t->val)){
	    	    dst << "-";
	} else {
	  // TODO external function
	    cloog_int_print(dst, t->val);
	    	    dst << "*";
	}
	if (group){
	    	    dst << "(";
	}
	pprint_expr(i, dst, t->var);
	if (group){
	    	    dst << ")";
	}
    } else
	cloog_int_print(dst, t->val);
}

void pprint_sum(struct cloogoptions *opt, std::stringstream& dst, struct clast_reduction *r)
{
    int i;
    struct clast_term *t;

    assert(r->n >= 1);
    assert(r->elts[0]->type == clast_expr_term);
    t = (struct clast_term *) r->elts[0];
    pprint_term(opt, dst, t);

    for (i = 1; i < r->n; ++i) {
	assert(r->elts[i]->type == clast_expr_term);
	t = (struct clast_term *) r->elts[i];
	if (cloog_int_is_pos(t->val)){
	    	    dst << "+";
	}
	pprint_term(opt, dst, t);
    }
}

void pprint_binary(struct cloogoptions *i, std::stringstream& dst, struct clast_binary *b)
{
    const char *s1 = NULL, *s2 = NULL, *s3 = NULL;
    int group = b->LHS->type == clast_expr_red &&
		((struct clast_reduction*) b->LHS)->n > 1;
    if (i->language == CLOOG_LANGUAGE_FORTRAN) {
	switch (b->type) {
	case clast_bin_fdiv:
	    s1 = "FLOOR(REAL(", s2 = ")/REAL(", s3 = "))";
	    break;
	case clast_bin_cdiv:
	    s1 = "CEILING(REAL(", s2 = ")/REAL(", s3 = "))";
	    break;
	case clast_bin_div:
	    if (group)
		s1 = "(", s2 = ")/", s3 = "";
	    else
		s1 = "", s2 = "/", s3 = "";
	    break;
	case clast_bin_mod:
	    s1 = "MOD(", s2 = ", ", s3 = ")";
	    break;
	}
    } else {
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
    }
        dst << s1;
    pprint_expr(i, dst, b->LHS);
        dst << s2;
    cloog_int_print(dst, b->RHS);
        dst << s3;
}

void pprint_minmax_f(struct cloogoptions *info, std::stringstream& dst, struct clast_reduction *r)
{
    int i;
    if (r->n == 0)
	return;
        if ( r->type == clast_red_max ){
      dst << "MAX(";
    }else{
     dst <<  "MIN(";
    }
    pprint_expr(info, dst, r->elts[0]);
    for (i = 1; i < r->n; ++i) {
		dst << ",";
	pprint_expr(info, dst, r->elts[i]);
    }
        dst << ")";
}

// TODO make it std::max std::min aware
void pprint_minmax_c(struct cloogoptions *info, std::stringstream& dst, struct clast_reduction *r)
{
    int i;
    for (i = 1; i < r->n; ++i){
		if ( r->type == clast_red_max ) {
	  dst << "max(";
	}else{
	  dst << "min(";
	}
    }
    if (r->n > 0)
	pprint_expr(info, dst, r->elts[0]);
    for (i = 1; i < r->n; ++i) {
		dst << ",";
	pprint_expr(info, dst, r->elts[i]);
		dst << ")";
    }
}

void pprint_reduction(struct cloogoptions *i, std::stringstream& dst, struct clast_reduction *r)
{
    switch (r->type) {
    case clast_red_sum:
	pprint_sum(i, dst, r);
	break;
    case clast_red_min:
    case clast_red_max:
	if (r->n == 1) {
	    pprint_expr(i, dst, r->elts[0]);
	    break;
	}
	if (i->language == CLOOG_LANGUAGE_FORTRAN)
	    pprint_minmax_f(i, dst, r);
	else
	    pprint_minmax_c(i, dst, r);
	break;
    default:
	assert(0);
    }
}

void pprint_expr(struct cloogoptions *i, stringstream& dst, struct clast_expr *e)
{
    if (!e)
	return;
    switch (e->type) {
    case clast_expr_name:
	pprint_name(dst, (struct clast_name*) e);
	break;
    case clast_expr_term:
	pprint_term(i, dst, (struct clast_term*) e);
	break;
    case clast_expr_red:
	pprint_reduction(i, dst, (struct clast_reduction*) e);
	break;
    case clast_expr_bin:
	pprint_binary(i, dst, (struct clast_binary*) e);
	break;
    default:
	assert(0);
    }
}

void pprint_equation(struct cloogoptions *i, std::stringstream& dst, struct clast_equation *eq)
{
    pprint_expr(i, dst, eq->LHS);
    if (eq->sign == 0){
		dst << " == ";
    }else if (eq->sign > 0){
		dst << " >= ";
    }else{
		dst << " <= ";
    }
    pprint_expr(i, dst, eq->RHS);
}

void pprint_assignment(struct cloogoptions *i, stringstream& dst, 
			struct clast_assignment *a)
{
    if (a->LHS){
            dst << a->LHS << " = ";
    }
    pprint_expr(i, dst, a->RHS);
}


/**
 * pprint_osl_body function:
 * this function pretty-prints the OpenScop body of a given statement.
 * It returns 1 if it succeeds to find an OpenScop body to print for
 * that statement, 0 otherwise.
 * \param[in] options CLooG Options.
 * \param[in] dst     Output stream.
 * \param[in] u       Statement to print the OpenScop body.
 * \return 1 on success to pretty-print an OpenScop body for u, 0 otherwise.
 */
int pprint_osl_body(struct cloogoptions *options, std::stringstream& dst,
                    struct clast_user_stmt *u) {
  return 0;
}

/* pprint_parentheses_are_safer function:
 * this function returns 1 if it decides that it would be safer to put
 * parentheses around the clast_assignment when it is used as a macro
 * parameter, 0 otherwise.
 * \param[in] s Pointer to the clast_assignment to check.
 * \return 1 if we should print parentheses around s, 0 otherwise.
 */
static int pprint_parentheses_are_safer(struct clast_assignment * s) {
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

void replace_marker_with( int id , std::string& text, std::string replacement ){
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

void pprint_user_stmt(struct cloogoptions *options, std::stringstream& dst,
		       struct clast_user_stmt *u)
{
    int parenthesis_to_close = 0;
    struct clast_stmt *t;

    if (pprint_osl_body(options, dst, u))
      return;
    
    if (u->statement->name){
		dst << u->statement->name;
    }else{
		//dst << "S" << u->statement->number;
	
	int ctr = 0;
	for (t = u->substitutions; t; t = t->next) {
	  stringstream substitution;
	  assert(CLAST_STMT_IS_A(t, stmt_ass));
	  if (pprint_parentheses_are_safer((struct clast_assignment *)t)) {
	    	    substitution << "(";
	    parenthesis_to_close = 1;
	  }
	  pprint_assignment(options, substitution, (struct clast_assignment *)t);
	  if (t->next) {
	      if (parenthesis_to_close) {
				substitution << ")";
		parenthesis_to_close = 0;
	      }
	  }

	  // at this point the substitution is generated
	  replace_marker_with( ctr++, (*global_statement_texts)[u->statement->number-1], substitution.str() );

	}
	dst << (*global_statement_texts)[u->statement->number-1];
    }
#if 0
        dst << "(";
    for (t = u->substitutions; t; t = t->next) {
	assert(CLAST_STMT_IS_A(t, stmt_ass));
        if (pprint_parentheses_are_safer((struct clast_assignment *)t)) {
	  	  dst << "(";
          parenthesis_to_close = 1;
        }
	pprint_assignment(options, dst, (struct clast_assignment *)t);
	if (t->next) {
            if (parenthesis_to_close) {
	      	      dst << ")";
              parenthesis_to_close = 0;
            }
	    	    dst << ",";
        }
    }
    if (parenthesis_to_close){
            dst << ")";
    }
        dst << ")";
    if (options->language != CLOOG_LANGUAGE_FORTRAN){
            dst << ";";
    }
        dst << endl;
#endif
}

void pprint_guard(struct cloogoptions *options, std::stringstream& dst, int indent,
		   struct clast_guard *g)
{
    int k;
    if (options->language == CLOOG_LANGUAGE_FORTRAN){
		dst << "IF ";
    }else{
		dst << "if ";
    }
    if (g->n > 1){
		dst << "(";
    }
    for (k = 0; k < g->n; ++k) {
	if (k > 0) {
	    if (options->language == CLOOG_LANGUAGE_FORTRAN){
				dst << " .AND. ";
	    }else{
				dst << " && ";
	    }
	}
		dst << "(";
        pprint_equation(options, dst, &g->eq[k]);
		dst << ")";
    }
    if (g->n > 1){
		dst << ")";
    }
    if (options->language == CLOOG_LANGUAGE_FORTRAN){
		dst << " THEN" << endl;
    }else{
		dst << "{" << endl;
    }

    pprint_stmt_list(options, dst, indent + INDENT_STEP, g->then);

        for (int i = 0; i < indent; ++i){
      dst << " "; 
    }
    if (options->language == CLOOG_LANGUAGE_FORTRAN){
		dst << "END IF" << endl;
    }else{
		dst << "}";
    }
}

void pprint_indent( std::stringstream& dst, int indent ){
    for (int i = 0; i < indent; ++i){
      dst << " "; 
    }
}

void pprint_for(struct cloogoptions *options, std::stringstream& dst, int indent,
		 struct clast_for *f)
{
    if (options->language == CLOOG_LANGUAGE_C) {
        if (f->time_var_name) {
                        //        (f->time_var_name) ? f->time_var_name : "");
	    if ( f->time_var_name ) {
	      dst << "IF_TIME(" << f->time_var_name << " _start = cloog_util_rtclock());" << endl;
	    }
        }
        if ((f->parallel & CLAST_PARALLEL_OMP) && !(f->parallel & CLAST_PARALLEL_MPI)) {
#if 0
            if (f->LB) {
                		dst << "lbp=";
                pprint_expr(options, dst, f->LB);
                		dst << ";" << endl;
            }
            if (f->UB) {
                		pprint_indent( dst, indent );
                		dst << "ubp=";
                pprint_expr(options, dst, f->UB);
                		dst << ";" << endl;
            }
#endif
#if 0
            fprintf(dst, "#pragma omp parallel for%s%s%s%s%s%s\n",
                    (f->private_vars)? " private(":"",
                    (f->private_vars)? f->private_vars: "",
                    (f->private_vars)? ")":"",
                    (f->reduction_vars)? " reduction(": "",
                    (f->reduction_vars)? f->reduction_vars: "",
                    (f->reduction_vars)? ")": "");
#endif
	    dst << "#pragma omp parallel for ";
#if 0
	    // TODO reenable if we have private vars
	    if ( f->private_vars ) {
	      dst << "private(" << f->private_vars << ")";
	    }
#endif 
	    if ( f->reduction_vars ) {
	      dst << "reduction(" << f->reduction_vars << ")";
	    }
	    dst << endl;
            	    pprint_indent( dst, indent );
        }
        if ((f->parallel & CLAST_PARALLEL_VEC) && !(f->parallel & CLAST_PARALLEL_OMP)
               && !(f->parallel & CLAST_PARALLEL_MPI)) {
            if (f->LB) {
                		dst << "auto lbv=";
                pprint_expr(options, dst, f->LB);
                		dst << ";" << endl;
            }
            if (f->UB) {
                		pprint_indent( dst, indent );
                		dst << "auto ubv=";
                pprint_expr(options, dst, f->UB);
                		dst << ";" << endl;
            }
            	    pprint_indent( dst, indent );
	    dst << "#pragma ivdep" << endl;
            	    pprint_indent( dst, indent );
	    dst << "#pragma vector always" << endl;
            	    pprint_indent(dst, indent );
        }
        if (f->parallel & CLAST_PARALLEL_MPI) {
            if (f->LB) {
                		dst << "_lb_dist=";
                pprint_expr(options, dst, f->LB);
                		dst << ";" << endl;
            }
            if (f->UB) {
                		pprint_indent( dst, indent );
                		dst << "_ub_dist=";
                pprint_expr(options, dst, f->UB);
                		dst << ";" << endl;
            }
            	    pprint_indent( dst, indent );
            	    dst << "polyrt_loop_dist(_lb_dist, _ub_dist, nprocs, my_rank, &lbp, &ubp);" << endl;
            if (f->parallel & CLAST_PARALLEL_OMP) {
#if 0
                fprintf(dst, "#pragma omp parallel for%s%s%s%s%s%s\n",
                        (f->private_vars)? " private(":"",
                        (f->private_vars)? f->private_vars: "",
                        (f->private_vars)? ")":"",
                        (f->reduction_vars)? " reduction(": "",
                        (f->reduction_vars)? f->reduction_vars: "",
                        (f->reduction_vars)? ")": "");
#endif
		dst << "#pragma omp parallel for ";
		// renable this if we really have private vars
#if 0
		if ( f->private_vars ) {
		  dst << "private(" << f->private_vars << ")";
		}
#endif
		if ( f->reduction_vars ) {
		  dst << "reduction(" << f->reduction_vars << ")";
		}
		dst << endl;
            }
            	    pprint_indent( dst, indent );
        }

    }

    if (options->language == CLOOG_LANGUAGE_FORTRAN){
		dst << "DO  ";
    }else{
		dst << "for (";
    }

    if (f->LB) {
		dst << "auto " << f->iterator << "=";
        if (f->parallel & (CLAST_PARALLEL_OMP | CLAST_PARALLEL_MPI)) {
            	    //dst << "lbp";
	    pprint_expr(options, dst, f->LB);
        }else if (f->parallel & CLAST_PARALLEL_VEC){
            	    dst << "lbv";
        }else{
	pprint_expr(options, dst, f->LB);
        }
    } else if (options->language == CLOOG_LANGUAGE_FORTRAN)
	cloog_die("unbounded loops not allowed in FORTRAN.\n");

    if (options->language == CLOOG_LANGUAGE_FORTRAN){
		dst << ", ";
    }else{
		dst << ";";
    }

    if (f->UB) { 
	if (options->language != CLOOG_LANGUAGE_FORTRAN){
	    	    dst << f->iterator << "<=";
	}

        if (f->parallel & (CLAST_PARALLEL_OMP | CLAST_PARALLEL_MPI)) {
            	    //dst << "ubp";
	    pprint_expr(options, dst, f->UB);
        }else if (f->parallel & CLAST_PARALLEL_VEC){
            	    dst << "ubv";
        }else{
            pprint_expr(options, dst, f->UB);
        }
    }else if (options->language == CLOOG_LANGUAGE_FORTRAN)
	cloog_die("unbounded loops not allowed in FORTRAN.\n");

    if (options->language == CLOOG_LANGUAGE_FORTRAN) {
	if (cloog_int_gt_si(f->stride, 1)){
	    cloog_int_print(dst, f->stride);
	}
		dst << endl;
    }
    else {
	if (cloog_int_gt_si(f->stride, 1)) {
	    	    dst << ";" << f->iterator << "+=";
	    cloog_int_print(dst, f->stride);
	    	    dst << ") {" << endl;
      } else
		dst << ";++" << f->iterator << ") {" << endl;
    }

    pprint_stmt_list(options, dst, indent + INDENT_STEP, f->body);

        pprint_indent( dst, indent );
    if (options->language == CLOOG_LANGUAGE_FORTRAN){
		dst << "END DO" << endl;
    }else{
		dst << "}" << endl;
    }

    if (options->language == CLOOG_LANGUAGE_C) {
        if (f->time_var_name) {
                        //        (f->time_var_name) ? f->time_var_name : "",
            //        (f->time_var_name) ? f->time_var_name : "");
	    if ( f->time_var_name ) {
	      dst << "IF_TIME(" << f->time_var_name << " += cloog_util_rtclock() - " << f->time_var_name<< "_start);" << endl;
	    }
        }
    }
}

void pprint_stmt_list(struct cloogoptions *options, stringstream& dst, int indent,
		       struct clast_stmt *s)
{
    for ( ; s; s = s->next) {
	if (CLAST_STMT_IS_A(s, stmt_root))
	    continue;

		// TODO make small function for indentation
	for (int i = 0; i < indent; ++i){
	  dst << " "; 
	}
	if (CLAST_STMT_IS_A(s, stmt_ass)) {
	    pprint_assignment(options, dst, (struct clast_assignment *) s);
	    if (options->language != CLOOG_LANGUAGE_FORTRAN){
	      dst << ";";
	    }
	    	    dst << endl;
	} else if (CLAST_STMT_IS_A(s, stmt_user)) {
	    pprint_user_stmt(options, dst, (struct clast_user_stmt *) s);
	} else if (CLAST_STMT_IS_A(s, stmt_for)) {
	    pprint_for(options, dst, indent, (struct clast_for *) s);
	} else if (CLAST_STMT_IS_A(s, stmt_guard)) {
	    pprint_guard(options, dst, indent, (struct clast_guard *) s);
	} else if (CLAST_STMT_IS_A(s, stmt_block)) {
	    	    dst << "{" << endl;
	    pprint_stmt_list(options, dst, indent + INDENT_STEP, 
				((struct clast_block *)s)->body);
	    	    for (int i = 0; i < indent; ++i){
	      dst << " "; 
	    }
	    	    dst << "}" << endl;
	} else {
	    assert(0);
	}
    }
}


/******************************************************************************
 *                       Pretty Printing (dirty) functions                    *
 ******************************************************************************/

void clast_pprint(stringstream& foo, struct clast_stmt *root,
		  int indent, 
		  CloogOptions *options, 
		  vector<std::string>& statement_texts,
		  std::map<std::string,std::string>& call_texts
		  )
{
  global_statement_texts = &statement_texts;
  global_call_texts = &call_texts;
    pprint_stmt_list(options, foo, indent, root);
}


void clast_pprint_expr(struct cloogoptions *i, std::stringstream& dst, struct clast_expr *e)
{
    pprint_expr(i, dst, e);
}

} // end of namespace clast_cxx
