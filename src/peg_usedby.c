/*
 * Copyright (c) 1994-2015 Nineties Retro
 */

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include "pe_as_rd.h"
#include "peg_usedby.h"


void
peg_usedby_open(struct peg_usedby *map)
{
	map->data = 0;
	map->n_rules = 0;
}


void
peg_usedby_free_space(struct peg_usedby_node *data, size_t start, size_t end)
{
	int result= 1;
	for (; start <= end; start += 1) {
		result &= id_idxs_map_close(&data[start].map);
	}
	free(data);
	return result;
}



int
peg_usedby_allocate_space(struct peg_usedby *map, size_t n_rules)
{
	size_t i;
	struct peg_usedby_node *p;

	p = realloc(map->data, (n_rules+1) * sizeof(struct peg_usedby_node));
	if (p == 0)
		goto could_not_allocate;
	
	for (i= n_rules; i != map->n_rules; i -= 1) {
		if (!id_idxs_map_open(&p[i].map, map->error))
			goto could_not_initialise;
		p[i].def= pe_as_idx_bottom();
	}
	map->n_rules= n_rules;
	map->data= p;
	return 1;

could_not_initialise:
	(void)peg_usedby_free_space(p, i+1, n_rules);
could_not_allocate:
	return 0;
}



peg_usedby_public int
peg_usedby_n_rules(struct peg_usedby *map, size_t n_rules)
{
	return peg_usedby_allocate_space(map, n_rules);
}



peg_usedby_public pe_as_idx
peg_usedby_mark_as_def(struct peg_usedby *map, pe_as_id rule_name, pe_as_idx rule)
{
	size_t idx= pe_as_id_code(rule_name);
	if (idx > map->n_rules) {
		if (!peg_usedby_allocate_space(map, idx))
			goto could_not_allocate_space;
	}
	if (pe_as_idx_valid(map->data[idx].def))
		return map->data[idx].def;
	map->data[idx].def = rule;
	return pe_as_idx_bottom();
could_not_allocate_space:
	return rule;
}


peg_usedby_public int
peg_usedby_is_def(peg_usedby *map, pe_as_id rule_name)
{
	size_t idx= pe_as_id_code(rule_name);
	return pe_as_idx_valid(map->data[idx].def);
}


peg_usedby_public int
peg_usedby_is_used(peg_usedby *map, pe_as_id rule_name)
{
	size_t idx= pe_as_id_code(rule_name);
	return !id_idxs_map_is_empty(&map->data[idx].map);
}



peg_usedby_public int
peg_usedby_insert(peg_usedby * map, pe_as_id used_id, pe_as_idx used, pe_as_id usedby)
{
	size_t idx = pe_as_id_code(used_id);

	if (idx > map->n_rules) {
		if (!peg_usedby_allocate_space(map, idx))
			goto could_not_allocate_space;
	}

	if (!id_idxs_map_insert(&map->data[idx].map, usedby, used))
		goto could_not_insert_info;

	return 1;

could_not_insert_info:
could_not_allocate_space:
	return 0;
}



int
peg_usedby_dump(struct peg_usedby *map, struct pe_as_rd *grammar, FILE *sink)
{
	size_t idx;

	if (fprintf(sink, "(usedby") == EOF)
		goto could_not_write;
	for (idx= 1; idx <= map->n_rules; idx += 1) {
		if (fprintf(sink, "\n  (rule %s", pe_as_rd_id_lit(grammar, size_t2pe_as_id(idx))) == EOF)
			goto could_not_write;
		if (!id_idxs_map_dump(&map->data[idx].map, grammar, sink))
			return 0;
		if (fprintf(sink, ")") == EOF)
			goto could_not_write;
	}
	if (fprintf(sink, ")\n") == EOF)
		goto could_not_write;
	return 1;

 could_not_write:
	pe_error_register_code(map->error, pe_error_file_io);
	return 0;
}



int
peg_usedby_close(peg_usedby * map)
{
	return peg_usedby_free_space(map->data, 1, map->n_rules);
}


/* eof */
