#ifndef pe_grammar_h
#define pe_grammar_h

/*
 * Copyright (c) 1994-2015 Nineties Retro
 */

/* XH: stdio.h */
/* XH: pe_as_lit_kw_map.h */

#include <stddef.h>
#include "pe_as_rd.h"
#include "pe_as_lit_kw_map_rd.h"

enum pe_rule_type {
	pe_rule_type_undefined= 0,
	pe_rule_type_id,   /* literal identifier, value = undefined */
	pe_rule_type_int,  /* literal integer, value = undefined */
	pe_rule_type_str,  /* literal string, value = undefined */
	pe_rule_type_kw,   /* keyword name, value = keyword code */
	pe_rule_type_rule, /* production name, value = production index */
	pe_rule_type_list, /* (list ...), value = list head */
	pe_rule_type_optional, /* (optional ...), value = optional head */
	pe_rule_type_or,       /* (or ...), value = or head  */
	pe_rule_type_unique    /* (unique ...), value = unique head */
};


struct pe_rule {
	enum pe_rule_type t;
	struct pe_rule    *next;
	union {
		size_t rule_index;
		pe_as_kw kw;
		struct pe_rule *child;
	} v;
};

typedef pe_as_idx pe_grammar_idx;

struct pe_grammar {
	struct pe_rule **rule;
	struct pe_rule *grammar;
	size_t n_rules;
	size_t n_nodes;
	struct pe_as_rd reader;
	pe_as_id start;
	struct pe_as_lit_kw_map_rd kws;
};

int
pe_grammar_open(struct pe_grammar *, char const *);

int
pe_grammar_pp(struct pe_grammar *, FILE *);

struct pe_as_lit_kw_map *
pe_grammar_kws(struct pe_grammar *);

int
pe_grammar_pp_rule_node(struct pe_grammar *, struct pe_rule *, FILE *);

struct pe_rule *
pe_grammar_dereference(struct pe_grammar *, struct pe_rule *);

void
pe_grammar_close(struct pe_grammar *);

#endif
