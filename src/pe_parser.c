/*
 * Copyright (c) 1994-2015 Nineties Retro
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>		/* for EXIT_FAILURE */
#include "pe_as_id.h"
#include "pe_hash.h"
#include "pe_as_lit_id_map.h"
#include "pe_grammar.h"
#include "pe_lexer.h"
#include "pe_parser.h"
#include "pe_rule_set.h"

static int
pe_parser_parse_and(struct pe_parser *, struct pe_rule *);

static int
pe_parser_parse_list(struct pe_parser *, struct pe_rule *, struct pe_rule *);

static int
pe_parser_parse_or(struct pe_parser *, struct pe_rule *);



static int
pe_parser_parse_optional(struct pe_parser *p, struct pe_rule *r)
{
	struct pe_rule *gr;
	int result;

	for (; r != 0; r= r->next) {
		switch (r->t) {
		case pe_rule_type_rule:
			gr = pe_grammar_dereference(p->grammar, r);
			if (gr->t == pe_rule_type_kw) {
				if ((p->lexeme.type == pe_lexeme_kw)
				    && pe_as_kw_eq(p->lexeme.v.kw, gr->v.kw)) {
					pe_lexer_lexeme(p->lexer, &p->lexeme);
					return pe_parser_parse_and(p, gr->next);
				}
			} else {
				result = pe_parser_parse_optional(p, gr);
				if (result != 0)
					return result;
			}
			break;

		case pe_rule_type_id:
			if (p->lexeme.type == pe_lexeme_id) {
				pe_lexer_lexeme(p->lexer, &p->lexeme);
				return 1;
			}
			break;

		case pe_rule_type_int:
			if (p->lexeme.type == pe_lexeme_int) {
				pe_lexer_lexeme(p->lexer, &p->lexeme);
				return 1;
			}
			break;

		case pe_rule_type_str:
			if (p->lexeme.type == pe_lexeme_str) {
				pe_lexer_lexeme(p->lexer, &p->lexeme);
				return 1;
			}
			break;

		case pe_rule_type_or:
			result = pe_parser_parse_optional(p, r->v.child);
			if (result != 0)
				return result;
			break;
			
		case pe_rule_type_list:
			result = pe_parser_parse_list(p, r->v.child, r->v.child);
			if (result != 0)
				return result;
			break;
			
		default:
			abort();
		}
	}
	return 0;
}




static const char *
pe_parser_kw_lit(struct pe_parser *p, pe_as_kw kw)
{
	size_t len;

	return pe_as_lit_kw_map_rd_lit(&p->grammar->kws, kw, &len);
}


static int
pe_parser_fatal_error(struct pe_parser *p)
{
	return -2;
}



static int
pe_parser_display_lexeme(struct pe_parser *p)
{
	switch (p->lexeme.error_type) {
	case pe_lexeme_eoi:
		fprintf(p->error_sink, "end of the file");
		break;

	case pe_lexeme_kw:
		if (p->lexeme.error == pe_lexeme_kw_unknown) {
			fprintf(p->error_sink, "(unknown) keyword \"");
		} else {
			fprintf(p->error_sink, "keyword \"");
		}
		fwrite(p->lexeme.str, 1, p->lexeme.len, p->error_sink);
		fputc('\'', p->error_sink);
		break;

	case pe_lexeme_int:
		fputs("integer \"", p->error_sink);
		fwrite(p->lexeme.str, 1, p->lexeme.len, p->error_sink);
		fputc('\"', p->error_sink);
		if (p->lexeme.error == pe_lexeme_int_overflow) {
			fprintf(p->error_sink, " which is too large (> %lu)", pe_as_int_max);
		}
		break;

	case pe_lexeme_id:
		fputs("identifier \"", p->error_sink);
		fwrite(p->lexeme.str, 1, p->lexeme.len, p->error_sink);
		fputc('\"', p->error_sink);
		break;

	case pe_lexeme_str:
		if (p->lexeme.error == pe_lexeme_str_unterminated) {
			fputs("unterminated string", p->error_sink);
		} else {
			fputs("string \"", p->error_sink);
			fwrite(p->lexeme.str, 1, p->lexeme.len, p->error_sink);
			fputc('\"', p->error_sink);
		}
		break;

	case pe_lexeme_eos:
		fputs(")", p->error_sink);
		break;

	case pe_lexeme_undefined:
		fputs("something", p->error_sink);
		break;

	default:
		break;
	}
	if (p->lexeme.error == pe_lexeme_bad_char) {
		char c = p->lexeme.v.bad_char;
		fprintf(p->error_sink, " that contains the illegal character \"%c\" (ASCII %d)", c, (int)c);
	}
	return -1;
}


static int
pe_parser_error_expected(struct pe_parser *p)
{
	if (p->lexeme.error == pe_lexeme_application_error)
		return pe_parser_fatal_error(p);
	if (!p->parse_error(p->lexeme.line))
		return -1;
	fprintf(p->error_sink, "\nfound: ");
	(void)pe_parser_display_lexeme(p);
	fprintf(p->error_sink, "\nexpected: ");
	return -1;
}



static int
pe_parser_error_eos(struct pe_parser *p)
{
	(void)pe_parser_error_expected(p);
	fprintf(p->error_sink, ")\n");
	return -1;
}



static int
pe_parser_error_keyword(struct pe_parser *p, pe_as_kw expected_kw)
{
	(void)pe_parser_error_expected(p);
	fprintf(p->error_sink, "keyword \"%s\"\n", pe_parser_kw_lit(p, expected_kw));
	return -1;
}



static int
pe_parser_error_int(struct pe_parser *p)
{
	(void)pe_parser_error_expected(p);
	fprintf(p->error_sink, "integer\n");
	return -1;
}



static int
pe_parser_error_id(struct pe_parser *p)
{
	(void)pe_parser_error_expected(p);
	fprintf(p->error_sink, "identifier\n");
	return -1;
}



static int
pe_parser_error_str(struct pe_parser *p)
{
	(void)pe_parser_error_expected(p);
	fprintf(p->error_sink, "string\n");
	return -1;
}



static int
pe_parser_error_or(struct pe_parser *p, struct pe_rule *or)
{
	(void)pe_parser_error_expected(p);
	pe_grammar_pp_rule_node(p->grammar, or, p->error_sink);
	fputc('\n', p->error_sink);
	return -1;
}



static int
pe_parser_error_list(struct pe_parser *p, struct pe_rule *list)
{
	(void)pe_parser_error_expected(p);
	pe_grammar_pp_rule_node(p->grammar, list, p->error_sink);
	fputc('\n', p->error_sink);
	return -1;
}



static int
pe_parser_error_eoi(struct pe_parser *p)
{
	(void)pe_parser_error_expected(p);
	fprintf(p->error_sink, "end of the file\n");
	return -1;
}



static int
pe_parser_error_unique(struct pe_parser *p, unsigned long line)
{
	if (p->lexeme.error == pe_lexeme_application_error)
		return pe_parser_fatal_error(p);

	if (!p->parse_error(p->lexeme.line))
		return 0;

	fprintf(p->error_sink, "\nfound: ");
	(void)pe_parser_display_lexeme(p);
	fprintf(p->error_sink, "\nprevious occurrence: line %lu\n", line);
	return -1;
}



static int
pe_parser_parse_unique(struct pe_parser *p, struct pe_rule *r,
		       struct pe_rule_set *uniques)
{
	struct pe_rule *gr;

	assert(r != (struct pe_rule *)0);
	assert(r->next == (struct pe_rule *)0);
	gr = pe_grammar_dereference(p->grammar, r);
	assert(gr->t == pe_rule_type_kw);

	if ((p->lexeme.type == pe_lexeme_kw)
	    &&  pe_as_kw_eq(p->lexeme.v.kw, gr->v.kw)) {
		unsigned long line = p->lexeme.line;
		int result = pe_rule_set_insert(uniques, gr->v.kw, &line);
		if (result >= 0) {
			return (result > 0) ? result : pe_parser_error_unique(p, line);
		} else {
			pe_lexer_lexeme(p->lexer, &p->lexeme);
			return pe_parser_parse_and(p, gr->next);
		}
	} else {
		return 0;
	}
}



static int
pe_parser_parse_list(struct pe_parser *p, struct pe_rule *start, struct pe_rule *r)
{
	int something_matched = 0;
	struct pe_rule *gr;
	struct pe_rule_set parsed_uniques;

	assert(start != (struct pe_rule *)0);
	assert(r != (struct pe_rule *)0);

	pe_rule_set_open(&parsed_uniques);
	for (;;) {
		switch (r->t) {
		case pe_rule_type_rule:
			gr = pe_grammar_dereference(p->grammar, r);
			if (gr->t == pe_rule_type_kw) {
				if ((p->lexeme.type == pe_lexeme_kw)
				    &&  pe_as_kw_eq(p->lexeme.v.kw, gr->v.kw)) {
					pe_lexer_lexeme(p->lexer, &p->lexeme);
					if ((something_matched= pe_parser_parse_and(p, gr->next)) < 0)
						goto the_end;
				}
			} else {
				/* parsing an alias ... */
				something_matched = pe_parser_parse_or(p, gr);
				if (something_matched < 0)
					goto the_end;
			}
			break;
      
		case pe_rule_type_id:
			if (p->lexeme.type == pe_lexeme_id) {
				pe_lexer_lexeme(p->lexer, &p->lexeme);
				something_matched= 1;
			} else {
				something_matched= 0;
			}
			break;
			
		case pe_rule_type_int:
			if (p->lexeme.type == pe_lexeme_int) {
				pe_lexer_lexeme(p->lexer, &p->lexeme);
				something_matched= 1;
			} else {
				something_matched= 0;
			}
			break;

		case pe_rule_type_str:
			if (p->lexeme.type == pe_lexeme_str) {
				pe_lexer_lexeme(p->lexer, &p->lexeme);
				something_matched= 1;
			} else {
				something_matched= 0;
			}
			break;
			
		case pe_rule_type_unique:
			something_matched = pe_parser_parse_unique(p, r->v.child, &parsed_uniques);
			if (something_matched < 0)
				goto the_end;
			break;
			
		default:
			abort();
		}

		if (something_matched) {
			something_matched= 0;
			start= r;
			/*
			 * This is done in preference to
			 *
			 *   r= start;
			 *
			 * on the assumption that the item just found
			 * will occur multiple times and so starting
			 * from the start again would waste time
			 * getting back to this point.  Should really
			 * do some analysis of files for various
			 * grammars to see whether this assumption is
			 * true.
			 */
		} else if ((start == r->next) ||
			   (p->lexeme.type == pe_lexeme_eos)) {
			something_matched= 1;
			goto the_end;
		} else {
			r= r->next;
		}
	}
the_end:
	pe_rule_set_close(&parsed_uniques);
	return something_matched;
}



static int
pe_parser_parse_or(struct pe_parser *p, struct pe_rule *r)
{
	struct pe_rule *gr;
	int result;

	for (; r != 0; r= r->next) {
		switch (r->t) {
		case pe_rule_type_rule:
			gr = pe_grammar_dereference(p->grammar, r);
			if (gr->t == pe_rule_type_kw) {
				if (p->lexeme.type == pe_lexeme_kw
				    &&  pe_as_kw_eq(p->lexeme.v.kw, gr->v.kw)) {
					pe_lexer_lexeme(p->lexer, &p->lexeme);
					return pe_parser_parse_and(p, gr->next);
				}
			} else if ((result= pe_parser_parse_or(p, gr)) != 0) {
				return result;
			}
			break;
		
		case pe_rule_type_id:
			if (p->lexeme.type == pe_lexeme_id) {
				pe_lexer_lexeme(p->lexer, &p->lexeme);
				return 1;
			}
			break;
			
		case pe_rule_type_int:
			if (p->lexeme.type == pe_lexeme_int) {
				pe_lexer_lexeme(p->lexer, &p->lexeme);
				return 1;
			}
			break;
			
		case pe_rule_type_str:
			if (p->lexeme.type == pe_lexeme_str) {
				pe_lexer_lexeme(p->lexer, &p->lexeme);
				return 1;
			}
			break;
			
		case pe_rule_type_or:
			result = pe_parser_parse_or(p, r->v.child);
			if (result != 0)
				return result;
			break;

		case pe_rule_type_list:
			result = pe_parser_parse_list(p, r->v.child, r->v.child);
			if (result != 0)
				return result;
			break;
			
		default:
			abort();
		}
	}
	return 0;
}



static int
pe_parser_parse_alias(struct pe_parser *p, struct pe_rule *r)
{
	struct pe_rule *gr;
	int result;

	assert(r != 0);
	assert(r->next == 0);

	switch (r->t) {
	case pe_rule_type_rule:
		gr = pe_grammar_dereference(p->grammar, r);
		if (gr->t == pe_rule_type_kw) {
			if ((p->lexeme.type == pe_lexeme_kw)
			    && pe_as_kw_eq(p->lexeme.v.kw, gr->v.kw)) {
				pe_lexer_lexeme(p->lexer, &p->lexeme);
				return pe_parser_parse_and(p, gr->next);
			} else {
				return pe_parser_error_keyword(p, gr->v.kw);
			}
		} else {
			/* parsing an alias ... */
			return pe_parser_parse_alias(p, gr);
		}
		break;

	case pe_rule_type_id:
		if (p->lexeme.type == pe_lexeme_id) {
			pe_lexer_lexeme(p->lexer, &p->lexeme);
			return 1;
		} else {
			return pe_parser_error_id(p);
		}
		break;

	case pe_rule_type_int:
		if (p->lexeme.type == pe_lexeme_int) {
			pe_lexer_lexeme(p->lexer, &p->lexeme);
			return 1;
		} else {
			return pe_parser_error_int(p);
		}
		break;

	case pe_rule_type_str:
		if (p->lexeme.type == pe_lexeme_str) {
			pe_lexer_lexeme(p->lexer, &p->lexeme);
			return 1;
		} else {
			return pe_parser_error_str(p);
		}
		break;

	case pe_rule_type_list:
		return pe_parser_parse_list(p, r->v.child, r->v.child);
		break;
		
	case pe_rule_type_or:
		result = pe_parser_parse_or(p, r->v.child);
		if (result == 0)
			return pe_parser_error_or(p, r);
		return result;
		break;

	case pe_rule_type_optional:
		return pe_parser_parse_optional(p, r->v.child);
		break;
		
	default:
		assert(0);
		return 0; /* keep pedantic but stupid compilers happy */
	}
}



static int
pe_parser_parse_and(struct pe_parser *p, struct pe_rule *r)
{
	int result;
	struct pe_rule *gr;

	for ( ; r != 0; r = r->next) {
		switch (r->t) {
		case pe_rule_type_rule:
			gr = pe_grammar_dereference(p->grammar, r);
			if (gr->t == pe_rule_type_kw) {
				if ((p->lexeme.type == pe_lexeme_kw)
				    &&  pe_as_kw_eq(p->lexeme.v.kw, gr->v.kw)) {
					pe_lexer_lexeme(p->lexer, &p->lexeme);
					result = pe_parser_parse_and(p, gr->next);
					if (result < 0)
						return result;
				} else {
					return pe_parser_error_keyword(p, gr->v.kw);
				}
			} else if ((result = pe_parser_parse_alias(p, gr)) < 0) {
				return result;
			}
			break;

		case pe_rule_type_id:
			if (p->lexeme.type == pe_lexeme_id) {
				pe_lexer_lexeme(p->lexer, &p->lexeme);
			} else {
				return pe_parser_error_id(p);
			}
			break;
			
		case pe_rule_type_int:
			if (p->lexeme.type == pe_lexeme_int) {
				pe_lexer_lexeme(p->lexer, &p->lexeme);
			} else {
				return pe_parser_error_int(p);
			}
			break;

		case pe_rule_type_str:
			if (p->lexeme.type == pe_lexeme_str) {
				pe_lexer_lexeme(p->lexer, &p->lexeme);
			} else {
				return pe_parser_error_str(p);
			}
			break;
			
		case pe_rule_type_list:
			result = pe_parser_parse_list(p, r->v.child, r->v.child);
			if (result < 0)
				return result;
			if ((r->next == 0)
			    && (p->lexeme.type != pe_lexeme_eos))
				return pe_parser_error_list(p, r);
			break;

		case pe_rule_type_or:
			result = pe_parser_parse_or(p, r->v.child);
			if (result < 0)
				return result;
			if (result == 0)
				return pe_parser_error_or(p, r);
			break;

		case pe_rule_type_optional:
			result = pe_parser_parse_optional(p, r->v.child);
			if (result < 0)
				return result;
			break;
			
		default:
			abort();
		}
	}

	assert(r == 0);

	switch (p->lexeme.type) {
	case pe_lexeme_eos:
		pe_lexer_lexeme(p->lexer, &p->lexeme);
		/* found an end-of-statement */
		return 1;
	case pe_lexeme_eoi:
		return 1;
	default:
		return pe_parser_error_eos(p);
	}
}




void
pe_parser_open(struct pe_parser *p, struct pe_lexer *l, struct pe_grammar *g)
{
	p->grammar = g;
	p->lexer = l;
	p->error_sink = stdout;
}



void
pe_parser_error_sink(struct pe_parser *p, int (*error)(unsigned long), FILE *error_sink)
{
	p->parse_error= error;
	p->error_sink= error_sink;
}



static int
pe_parser_xxx(struct pe_parser *p)
{
	struct pe_rule start;
	struct pe_rule *gr;

	start.t = pe_rule_type_rule;
	start.next = (struct pe_rule *)0;
	start.v.rule_index = pe_as_id_code(p->grammar->start);
	pe_lexer_lexeme(p->lexer, &p->lexeme);
	gr = pe_grammar_dereference(p->grammar, &start);
	if (gr->t == pe_rule_type_kw) {
		if ((p->lexeme.type == pe_lexeme_kw)
		    && pe_as_kw_eq(p->lexeme.v.kw, gr->v.kw)) {
			pe_lexer_lexeme(p->lexer, &p->lexeme);
			return pe_parser_parse_and(p, gr->next);
		} else {
			return pe_parser_error_keyword(p, gr->v.kw);
		}
	} else {
		return pe_parser_parse_alias(p, gr);
	}
}



int
pe_parser_parse(struct pe_parser *p)
{
	int result = pe_parser_xxx(p);

	if (result < 0) {
		return (result == -1) ? 1 : result;
	} else if (p->lexeme.type != pe_lexeme_eoi) {
		result= pe_parser_error_eoi(p);
		return (result == -1) ? 1 : result;
	} else {
		return 0;
	}
}



void
pe_parser_close(struct pe_parser *p)
{
}
