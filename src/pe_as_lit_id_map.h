#ifndef pe_as_lit_id_map_h
#define pe_as_lit_id_map_h

/*
 * Copyright (c) 1994-2015 Nineties Retro
 */

/* XH: stddef.h */
/* XH: pe_hash.h */
/* XH: pe_as_id.h */

struct pe_as_lit_id_map_node {
	struct pe_as_lit_id_map_node *next;
	char *full_id;
	char *cmp_id;
	size_t cmp_len;
	pe_as_id code;
};


struct pe_as_lit_id_map {
	struct pe_as_lit_id_map_node **id_table;
	size_t table_size;
	pe_as_id id_code;
};

int
pe_as_lit_id_map_open(struct pe_as_lit_id_map *);

struct pe_as_lit_id_map_node **
pe_as_lit_id_map_find(struct pe_as_lit_id_map *, size_t, char const *, int, pe_hash);

int
pe_as_lit_id_map_enter(struct pe_as_lit_id_map *m, size_t len, char const *id, int fix, struct pe_as_lit_id_map_node **n);

void
pe_as_lit_id_map_close(struct pe_as_lit_id_map *, void (*)(char *));

#endif
