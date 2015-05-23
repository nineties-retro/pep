/*
 * Copyright (c) 1994-2015 Nineties Retro
 */

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <errno.h>
#include "pe_as_kw.h"
#include "pe_hash.h"
#include "pe_as_lit_kw_map.h"

static int
pe_as_lit_kw_map_chareq(char a, char b)
{
	return (a & ~('a' - 'A')) == (b & ~('a' - 'A'));
}



static int
pe_as_lit_kw_map_streq(size_t al, char const *a, size_t bl, char const *b) 
{
	if (al != bl)
		return 0;
	while (al-- != 0) {
		if (!pe_as_lit_kw_map_chareq(*a++, *b++))
			return 0;
	}
	return 1;
}



struct pe_as_lit_kw_map_node **
pe_as_lit_kw_map_find(struct pe_as_lit_kw_map *t, size_t len, char const *kw, int fix, pe_hash hash)
{
	struct pe_as_lit_kw_map_node **node;
	struct pe_as_lit_kw_map_node  *l;
	size_t cmp_len;
	char const *cmp_kw;

	assert(len < 256);
	
	cmp_len = len-fix;
	cmp_kw = kw+fix;
	node = &t->kw_table[hash%t->table_size];
	for (l = *node; l != 0; node = &l->next, l = l->next) {
		if (pe_as_lit_kw_map_streq(cmp_len, cmp_kw, l->cmp_len, l->cmp_kw))
			return node;
	}
	return node;
}


int
pe_as_lit_kw_map_enter(struct pe_as_lit_kw_map *m, size_t len,
		       char const *kw, int fix,
		       struct pe_as_lit_kw_map_node **n)
{
	struct pe_as_lit_kw_map_node *l;

	l = malloc(sizeof(*l));
	if (l == 0)
		return ENOMEM;
	l->full_kw = (char *)kw;
	l->cmp_kw = (char *)(kw+fix+1);
	l->cmp_len = len-fix;
	l->next = *n;
	m->kw_code = pe_as_kw_succ(m->kw_code);
	*n = l;
	return 0;
}



int
pe_as_lit_kw_map_open(struct pe_as_lit_kw_map *m)
{
	size_t i;

	m->kw_code = pe_as_kw_bottom();
	m->table_size = 101;
	m->kw_table = malloc(m->table_size * sizeof(struct pe_as_lit_kw_map_node *));
	if (m->kw_table == 0)
		return ENOMEM;
	for (i= 0; i < m->table_size; i++)
		m->kw_table[i] = 0;
	return 0;
}



void
pe_as_lit_kw_map_close(struct pe_as_lit_kw_map *m, void (*delete_string)(char *))
{
	size_t i;
	struct pe_as_lit_kw_map_node *n, *p;

	for (i = 0; i < m->table_size; i++) {
		n = m->kw_table[i];
		while (n != 0) {
			p = n;
			n = n->next;
			if (delete_string)
				delete_string(p->full_kw);
			free(p);
		}
	}
	free(m->kw_table);
}
