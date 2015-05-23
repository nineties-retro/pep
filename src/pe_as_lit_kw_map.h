#ifndef pe_as_lit_kw_map_h
#define pe_as_lit_kw_map_h

/*
 * Copyright (c) 1994-2015 Nineties Retro
 */

#include <stddef.h>
#include <stdio.h>
/* XH: pe_hash.h */
/* XH: pe_as_kw.h */

struct pe_as_lit_kw_map_node {
	struct pe_as_lit_kw_map_node *next;
	char     *full_kw;
	char     *cmp_kw;
	size_t    cmp_len;
	pe_as_kw  code;
};


struct pe_as_lit_kw_map {
	struct pe_as_lit_kw_map_node **kw_table;
	size_t table_size;
	pe_as_kw kw_code;
};

int
pe_as_lit_kw_map_open(struct pe_as_lit_kw_map *);

struct pe_as_lit_kw_map_node **
pe_as_lit_kw_map_find(struct pe_as_lit_kw_map *, size_t, char const *, int, pe_hash);

int
pe_as_lit_kw_map_enter(struct pe_as_lit_kw_map *, size_t, char const *, int fix, struct pe_as_lit_kw_map_node **);

void
pe_as_lit_kw_map_close(struct pe_as_lit_kw_map *, void (*)(char *));

#endif
