/*
 * Copyright (c) 1994-2015 Nineties Retro
 */

#include <assert.h>		/* assert */
#include <errno.h>		/* ENOMEM */
#include <stddef.h>
#include <stdio.h>		/* fputc, fputs */
#include <stdlib.h>
#include <string.h>		/* memset */
#include "pe_kw.h"
#include "pe_as_id.h"
#include "pe_as_kw.h"
#include "pe_hash.h"
#include "pe_as_lit_kw_map.h"
#include "pe_grammar.h"
#include "pe_as_str_lit_map_rd.h"

/*
 * The grammar has a header which consists of one node
 * the "grammar" node and the (start xxx) node.  This gives rise to :-
 */
#define pe_grammar_n_nodes_in_header 3


/* 
 * There are three builtin rules and the have the form :-
 *
 *  (identifier xxx)
 *  (string yyy)
 *  (integer zzz)
 *
 * These give rise to the following :-
 */
#define pe_grammar_n_builtin_rules 3
#define pe_grammar_n_nodes_in_a_builtin_rule 2


#define pe_grammar_n_builtin_nodes \
  ((pe_grammar_n_builtin_rules*pe_grammar_n_nodes_in_a_builtin_rule)+pe_grammar_n_nodes_in_header)

/*
 * Each rule consists of a keyword indicating the type of the rule
 * and the rule name.
 */
#define pe_grammar_n_nodes_in_rule_header 2



int
pe_grammar_allocate_grammar_nodes(struct pe_grammar *g, size_t n_rules,
				  size_t n_nodes)
{
	size_t i;
	struct pe_rule **rule;
	struct pe_rule *grammar = malloc(n_nodes*sizeof(struct pe_rule));
	int err;

	if (grammar == 0)
		return ENOMEM;
	for (i= 0; i < n_nodes; i++)
		grammar[i].t = pe_rule_type_undefined;
	rule = malloc((n_rules+1)*sizeof(struct pe_rule *));
	if (rule == 0) {
		err = ENOMEM;
		goto could_not_create_space_for_index;
	}

	memset(rule, 0, (n_rules+1)*sizeof(struct pe_rule *));
	g->grammar = grammar;
	g->rule = rule;
	g->n_rules = n_rules;
	g->n_nodes = n_nodes;
	return 0;

could_not_create_space_for_index:
	free(grammar);
	return err;
}



void
pe_grammar_delete_grammar(struct pe_grammar *g)
{
	free(g->grammar);
	free(g->rule);
}



struct pe_rule *
pe_grammar_enter_rule(struct pe_grammar *g, pe_as_idx p, 
		      struct pe_rule *r, struct pe_rule **link,
		      struct pe_rule *next)
{
	struct pe_as_rd *raw_grammar = &g->reader;
	pe_as_nt nt;
	pe_as_id id;
	
	for ( ; pe_as_idx_valid(p); p = pe_as_rd_next(raw_grammar, p)) {
		*link= r;
		r->next = next;
		link = &r->next;

		assert(r <= g->grammar + g->n_nodes);
		assert(r->t == pe_rule_type_undefined);
		nt = pe_as_rd_type(raw_grammar, p);
		switch (nt) {
		case pe_as_nt_kw:
			switch (pe_as_kw2size_t(pe_as_rd_kw(raw_grammar, p))) {
			case pe_kw_list:
				r->t = pe_rule_type_list;
				r = pe_grammar_enter_rule(g, pe_as_rd_child(raw_grammar, p), r+1, &r->v.child, r+1);
				break;
			case pe_kw_or:
				r->t = pe_rule_type_or;
				r = pe_grammar_enter_rule(g,  pe_as_rd_child(raw_grammar, p), r+1, &r->v.child, 0);
				break;
			case pe_kw_optional:
				r->t = pe_rule_type_optional;
				r = pe_grammar_enter_rule(g, pe_as_rd_child(raw_grammar, p), r+1, &r->v.child, 0);
				break;
			case pe_kw_unique:
				r->t = pe_rule_type_unique;
				r = pe_grammar_enter_rule(g, pe_as_rd_child(raw_grammar, p), r+1, &r->v.child, 0);
				break;
			}
			break;

		case pe_as_nt_id:
			id = pe_as_rd_id(raw_grammar, p);
			size_t code = pe_as_id_code(id);
			switch (code) {
			case 1:
				r->t = pe_rule_type_id;
				break;
			case 2:
				r->t = pe_rule_type_str;
				break;
			case 3:
				r->t = pe_rule_type_int;
				break;
			default:
				r->t= pe_rule_type_rule;
			}
			r->v.rule_index = code;
			r += 1;
			break;
		default:
			abort();
		}
	}
	return r;
}



void
pe_grammar_create_keyword_rule(struct pe_grammar *g, pe_as_idx body, struct pe_rule *n)
{
	struct pe_as_rd *raw_grammar = &g->reader;
	pe_as_str name = pe_as_rd_str(raw_grammar, body);
	size_t len;
	char const *str_lit= pe_as_rd_str_lit_len(raw_grammar, name, &len);
	pe_as_kw kw_code = pe_as_lit_kw_map_rd_kw(&g->kws, len, str_lit);

	assert(!pe_as_kw_eq(kw_code, pe_as_kw_bottom()));
	assert(n->t == pe_rule_type_undefined);
	n->t = pe_rule_type_kw;
	n->next = 0;
	n->v.kw = kw_code;
}



int
pe_grammar_pp_rule_nodes(struct pe_grammar *g, 
			 struct pe_rule *r, struct pe_rule *end,
			 FILE *output)
{
	do {
		int err = pe_grammar_pp_rule_node(g, r, output);
		if (err)
			return err;
		r = r->next;
	} while (r != end);
	return 0;
}



size_t
pe_grammar_count_comment_nodes(struct pe_as_rd *grammar, pe_as_idx p)
{
	size_t n= 0;

	for (; pe_as_idx_valid(p); p = pe_as_rd_next(grammar, p)) {
		assert(pe_as_rd_type(grammar, p) == pe_as_nt_str);
		n += 1;
	}
	return n;
}



void
pe_grammar_count_rules_and_comment_nodes(struct pe_as_rd *grammar,
					 pe_as_idx p, size_t *n_rules,
					 size_t *n_comment_nodes)
{
	for (; pe_as_idx_valid(p); p= pe_as_rd_next(grammar, p)) {
		switch (pe_as_kw2size_t(pe_as_rd_kw(grammar, p))) {
		case pe_kw_keyword:
		case pe_kw_alias:
			*n_rules += 1;
			break;

		case pe_kw_comment:
			*n_comment_nodes += 1 + pe_grammar_count_comment_nodes(grammar, pe_as_rd_child(grammar, p));
			break;
		}
	}
}



int
pe_grammar_init(struct pe_grammar *g)
{
	struct pe_as_rd *raw_grammar = &g->reader;
	pe_as_idx root = pe_as_rd_root(raw_grammar);
	pe_as_idx identifier = pe_as_rd_child(raw_grammar, root);
	pe_as_idx string = pe_as_rd_next(raw_grammar, identifier);
	pe_as_idx integer = pe_as_rd_next(raw_grammar, string);
	pe_as_idx start = pe_as_rd_next(raw_grammar, integer);
	pe_as_idx p = pe_as_rd_next(raw_grammar, start);
	struct pe_rule *n;
	int err;

	{
		size_t      n_rules= pe_as_rd_n_id(raw_grammar);
		size_t      n_user_rules= 0;
		size_t      n_nodes= pe_as_rd_n_node(raw_grammar);
		size_t      n_comment_nodes= 0;
		size_t      n_user_rule_nodes;
		/*
		 * If it wasn't for the draconian rule that the no
		 * memory is allocated unless it is absolutely
		 * required, there would be no need to count the
		 * number of comment nodes since it would suffice to
		 * allocate, but not use, space for them.
		 */
		pe_grammar_count_rules_and_comment_nodes(raw_grammar, p, &n_user_rules, &n_comment_nodes);
		assert(n_rules == n_user_rules + pe_grammar_n_builtin_rules);
		n_user_rule_nodes = n_nodes - pe_grammar_n_builtin_nodes - n_user_rules*pe_grammar_n_nodes_in_rule_header - n_comment_nodes;

		err = pe_grammar_allocate_grammar_nodes(g, n_rules, n_user_rule_nodes);
		if (err)
			goto could_not_allocate_grammar_nodes;

		g->start = pe_as_rd_id(raw_grammar, pe_as_rd_child(raw_grammar, start));
		assert(pe_as_id_code(g->start) <= n_rules);
	}

	n=  g->grammar;
	for ( ; pe_as_idx_valid(p); p = pe_as_rd_next(raw_grammar, p)) {
		size_t kw = pe_as_kw2size_t(pe_as_rd_kw(raw_grammar, p));

		if (kw != pe_kw_comment) {
			pe_as_idx head = pe_as_rd_child(raw_grammar, p);
			pe_as_idx body = pe_as_rd_next(raw_grammar, head);
			size_t i = pe_as_id_code(pe_as_rd_id(raw_grammar, head));
			
			assert(g->rule[i] == 0);
			assert(n <= g->grammar + g->n_nodes);
			switch(kw) {
			case pe_kw_keyword:
				pe_grammar_create_keyword_rule(g, body, n);
				g->rule[i]= n;
				n = pe_grammar_enter_rule(g, pe_as_rd_next(raw_grammar, body), n+1, &n->next, 0);
				break;
				
			case pe_kw_alias:
				n = pe_grammar_enter_rule(g, body, n, &g->rule[i], 0);
				break;
				
			default:
				break;
			}
		}
	}
	
	assert(n == g->grammar + g->n_nodes);
	return 0;

could_not_allocate_grammar_nodes:
	return err;
}



int
pe_grammar_pp_rule_node(struct pe_grammar *g, struct pe_rule *r, FILE *output)
{
	size_t len;
	char const *id;
	int err;

	switch (r->t) {
	case pe_rule_type_rule:
	case pe_rule_type_id:
	case pe_rule_type_int:
	case pe_rule_type_str:
		id = pe_as_id_lit_map_rd_id(&g->reader.ids, size_t2pe_as_id(r->v.rule_index), &len);
		if (fputc(' ', output) == EOF)
			return EIO;
		if (fputs(id, output) == EOF)
			return EIO;
	break;
	case pe_rule_type_list:
		if (fputs(" (list", output) == EOF)
			return EIO;
		err = pe_grammar_pp_rule_nodes(g, r->v.child, r->v.child, output);
		if (err)
			return err;
		if (fputc(')', output) == EOF)
			return EIO;
		break;
	case pe_rule_type_or:
		if (fputs(" (or", output) == EOF)
			return EIO;
		err = pe_grammar_pp_rule_nodes(g, r->v.child, 0, output);
		if (err)
			return err;
		if (fputc(')', output) == EOF)
			return EIO;
		break;
	case pe_rule_type_unique:
		if (fputs(" (unique", output) == EOF)
			return EIO;
		err = pe_grammar_pp_rule_nodes(g, r->v.child, 0, output);
		if (err)
			return err;
		if (fputc(')', output) == EOF)
			return EIO;
		break;
	case pe_rule_type_optional:
		if (fputs(" (optional", output) == EOF)
			return EIO;
		err = pe_grammar_pp_rule_nodes(g, r->v.child, 0, output);
		if (err)
			return err;
		if (fputc(')', output) == EOF)
			return EIO;
		break;
	case pe_rule_type_kw:
		id = pe_as_lit_kw_map_rd_lit(&g->kws, r->v.kw, &len);
		if (fputs(" \"", output) == EOF)
			return EIO;
		if (fputs(id, output) == EOF)
			return EIO;
		if (fputc('"', output) == EOF)
			return EIO;
		break;
	default:
		abort();
	}
	return 0;
}



int
pe_grammar_pp(struct pe_grammar *g, FILE *output)
{
	size_t i;
	size_t len;
	char const *id;
	int err;

	for (i = 1; i <= g->n_rules; i += 1) {
		struct pe_rule *r = g->rule[i];
		if (!r)
			continue;
		id = pe_as_id_lit_map_rd_id(&g->reader.ids, size_t2pe_as_id(i), &len);
		if (fputc('(', output) == EOF)
			return EIO;
		if (fputs((r->t == pe_rule_type_kw) ? "keyword " : "alias ", output) == EOF)
			return EIO;
		if (fputs(id, output) == EOF)
			return EIO;
		err = pe_grammar_pp_rule_nodes(g, r, 0, output);
		if (err)
			return err;
		if (fputs(")\n", output) == EOF)
			return EIO;
	}
	return 0;
}



int
pe_grammar_open(struct pe_grammar *g, char const *language)
{
	char keyword_file[PATH_MAX];
	int err;

	pe_as_rd_open(&g->reader);
	err = pe_as_rd_data(&g->reader, language);
	if (err)
		goto could_not_read_grammar;
	err = snprintf(keyword_file, sizeof keyword_file, "%skw", language);
	if ((err < 0) || (err >= sizeof keyword_file)) {
		err = ENAMETOOLONG;
		goto could_not_create_keyword_name;
	}
	err = pe_as_lit_kw_map_rd_open(&g->kws, keyword_file);
	if (err)
		goto could_not_open_keyword_map;
	err = pe_grammar_init(g);
	if (err)
		goto could_not_initialise;
	return err;

could_not_initialise:
	pe_as_lit_kw_map_rd_close(&g->kws);
could_not_open_keyword_map:
could_not_create_keyword_name:
could_not_read_grammar:
	pe_as_rd_close(&g->reader);
	return err;
}


struct pe_rule *
pe_grammar_dereference(struct pe_grammar *g, struct pe_rule *r)
{
	size_t index;

	assert(r->t == pe_rule_type_rule);
	index = r->v.rule_index;
	assert(index <= g->n_rules);
	return g->rule[index];
}


struct pe_as_lit_kw_map *
pe_grammar_kws(struct pe_grammar *g)
{
	return &g->kws.map;
}



void
pe_grammar_close(struct pe_grammar *g)
{
	pe_grammar_delete_grammar(g);
	pe_as_lit_kw_map_rd_close(&g->kws);
	pe_as_rd_close(&g->reader);
}
