/*
 * Copyright (c) 1994-2015 Nineties Retro
 */

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <errno.h>
#include "pe_as_id.h"
#include "pe_hash.h"
#include "pe_as_lit_id_map.h"

int
pe_as_lit_id_map_chareq(char a, char b)
{
	return (a & ~('a' - 'A')) == (b & ~('a' - 'A'));
}



int
pe_as_lit_id_map_streq(size_t al, char const *a, size_t bl, char const *b) 
{
	if (al != bl)
		return 0;
	while (al-- != 0) {
		if (!pe_as_lit_id_map_chareq(*a++, *b++))
			return 0;
	}
	return 1;
}



struct pe_as_lit_id_map_node **
pe_as_lit_id_map_find(struct pe_as_lit_id_map *t, size_t len, char const *id, int fix, pe_hash hash)
{
	struct pe_as_lit_id_map_node **node;
	struct pe_as_lit_id_map_node *l;
	size_t cmp_len;
	char const *cmp_id;
	
	assert(len < 256);
	cmp_len = len-fix;
	cmp_id = id+fix;
	node = &t->id_table[hash%t->table_size];
	for (l = *node; l != 0; node = &l->next, l = l->next) {
		if (pe_as_lit_id_map_streq(cmp_len, cmp_id, l->cmp_len, l->cmp_id))
			return node;
	}
	return node;
}


int
pe_as_lit_id_map_enter(struct pe_as_lit_id_map *m, size_t len, char const *id, int fix, struct pe_as_lit_id_map_node **n)
{
	struct pe_as_lit_id_map_node *l = malloc(sizeof(*l));

	if (l == 0)
		return ENOMEM;
	l->full_id = (char *)id;
	l->cmp_id = (char *)(id+fix+1);
	l->cmp_len = len-fix;
	l->next = *n;
	m->id_code = pe_as_id_succ(m->id_code);
	*n= l;
	return 0;
}



int
pe_as_lit_id_map_open(struct pe_as_lit_id_map *m)
{
	size_t i;

	m->id_code = pe_as_id_bottom();
	m->table_size = 10007;
	m->id_table = malloc(m->table_size * sizeof(struct pe_as_lit_id_map_node *));
	if (m->id_table == 0)
		return ENOMEM;
	for (i = 0; i < m->table_size; i++)
		m->id_table[i] = 0;
	return 0;
}



void
pe_as_lit_id_map_close(struct pe_as_lit_id_map *m,
		       void (*delete_string)(char *))
{
	size_t i;

	for (i = 0; i < m->table_size; i++) {
		struct pe_as_lit_id_map_node *n = m->id_table[i];

		while (n != 0) {
			struct pe_as_lit_id_map_node *p = n;

			n = n->next;
			if (delete_string)
				delete_string(p->full_id);
			free(p);
		}
	}
	free(m->id_table);
}
