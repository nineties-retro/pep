/*
 * Copyright (c) 1994-2015 Nineties Retro
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include "pe_as_kw.h"
#include "pe_as_lit_kw_map_rd.h"

pe_as_kw
pe_as_lit_kw_map_rd_kw(struct pe_as_lit_kw_map_rd *m, size_t len, char const *kw)
{
	struct pe_as_lit_kw_map_node ** n;
	pe_hash hash;
	int fix;

	assert(kw != (char const *)0);
	assert(len != 0);

	fix =  (kw[0] == '&') ? 1 : 0;
	hash = pe_hash_lit(len-fix, kw+fix);
	n = pe_as_lit_kw_map_find(&m->map, len, kw, fix, hash);
	if (*n == 0) {
		return pe_as_kw_bottom();
	} else {
		return (*n)->code;
	}
}




static int
pe_as_lit_kw_map_rd_enter_kw(struct pe_as_lit_kw_map_rd *m,
			     char const *kw, int fix,
			     pe_as_kw code, pe_hash hash)
{
	size_t len;
	struct pe_as_lit_kw_map_node ** n, *l;

	len = (char)kw[0];
	n = pe_as_lit_kw_map_find(&m->map, len, kw+1, fix, hash);
	l = *n;
	if (l == 0) {
		int err = pe_as_lit_kw_map_enter(&m->map, len, kw, fix, n);
		if (err)
			return err;
		(*n)->code= code;
	}
	return 0;
}


static int
pe_as_lit_kw_map_rd_read(struct pe_as_lit_kw_map_rd *m)
{
	size_t n_kw= pe_as_kw_lit_map_rd_n_kws(&m->kws);
	char const ** kws= m->kws.kws;
	int err = 0;

	for ( ; n_kw != 0; n_kw -= 1) {
		char const * lit = kws[n_kw];
		const int fix = (lit[1] == '&') ? 1 : 0;
		const size_t len = (size_t)lit[0];
		const pe_hash h = pe_hash_lit(len-fix, lit+fix+1);
		const pe_as_kw kw = size_t2pe_as_kw(n_kw);
		int err = pe_as_lit_kw_map_rd_enter_kw(m, lit, fix, kw, h);

		if (err)
			return err;
	}
	return err;
}

int
pe_as_lit_kw_map_rd_open(struct pe_as_lit_kw_map_rd *m, char const *file_name)
{
	int err;

	err = pe_as_lit_kw_map_open(&m->map);
	if (err)
		goto could_not_open_lit_kw_map;
	err = pe_as_kw_lit_map_rd_open(&m->kws, file_name);
	if (err)
		goto could_not_open_kw_lit_map_reader;
	err = pe_as_lit_kw_map_rd_read(m);
	if (err)
		goto could_not_read_lit_kw_map;
	return err;

could_not_read_lit_kw_map:
	(void)pe_as_kw_lit_map_rd_close(&m->kws);
could_not_open_kw_lit_map_reader:
	(void)pe_as_lit_kw_map_close(&m->map, 0);
could_not_open_lit_kw_map:
	return err;
}



void
pe_as_lit_kw_map_rd_close(struct pe_as_lit_kw_map_rd * m)
{
	pe_as_lit_kw_map_close(&m->map, 0);
	pe_as_kw_lit_map_rd_close(&m->kws);
}
