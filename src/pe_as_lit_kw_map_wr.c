/*
 * Copyright (c) 1994-2015 Nineties Retro
 */

/*
 * pe_as_lit_kw_map_wr is a mapping from a literal string representing
 * keyword to the keyword code that is persistent, that is the mapplets
 * inserted into a map are written to some non-volatile memory.
 *
 * The interface is very simple, there are only three procedures: open
 * kw, and close.
 */

#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pe_as_kw.h"
#include "pe_hash.h"
#include "pe_as_lit_kw_map_wr.h"

/*
 * The awkward +1 and +2s in the following are due to the fact that
 * the first character of an id encodes the length of the id and the
 * fact that ids are null terminated.
 */
int
pe_as_lit_kw_map_wr_kw(struct pe_as_lit_kw_map_wr *m, size_t len,
		       char const *id, int fix, pe_hash hash, pe_as_kw *code)
{
	struct pe_as_lit_kw_map_node **n, *l;
	int err;

	if (len >= 256)
		return ENAMETOOLONG;
	n = pe_as_lit_kw_map_find(&m->map, len, id, fix, hash);
	l = *n;
	if (l == 0) {
		size_t n_written;
		char *lit = malloc(len+2);
		
		if (lit == 0) {
			err = ENOMEM;
			goto could_not_create_node;
		}
		lit[0] = len;
		memcpy(&lit[1], id, len);
		lit[len+1]= '\0';

		err = pe_as_lit_kw_map_enter(&m->map, len, lit, fix, n);
		if (err)
			goto could_not_add_node;
		n_written = fwrite(lit, len+2, 1, m->sink);
		if (n_written != 1)
			goto could_not_write_node;
		m->data_size += len+2;
		*code = (*n)->code = m->map.kw_code;
		return 0;
	could_not_write_node:
	could_not_add_node:
		free(lit);
	could_not_create_node:
		return err;
	} else {
		*code= l->code;
		return 0;
	}
}


static void
pe_as_lit_kw_map_wr_delete_strings(char *s)
{
	free(s);
}


int
pe_as_lit_kw_map_wr_open(struct pe_as_lit_kw_map_wr *m, char const *file_name)
{
	size_t n_written;
	int err;

	err = pe_as_lit_kw_map_open(&m->map);
	if (err)
		goto could_not_open_map;

	m->sink = fopen(file_name, "w+b");
	if (m->sink == 0) {
		err = errno;
		goto could_not_open_file;
	}

	m->data_size= 0;
	n_written = fwrite(&m->data_size, sizeof(unsigned long), 1, m->sink);
	if (n_written != 1) {
		err = errno;
		goto could_not_write_size;
	}

	n_written = fwrite(&m->map.kw_code, sizeof(pe_as_kw), 1, m->sink);
	if (n_written != 1) {
		err = errno;
		goto could_not_write_id_code;
	}
	return err;

could_not_write_id_code:
could_not_write_size:
	(void)fclose(m->sink);
could_not_open_file:
	pe_as_lit_kw_map_close(&m->map, pe_as_lit_kw_map_wr_delete_strings);
could_not_open_map:
	return err;
}



/*
 * `pe_as_lit_kw_map_wr_close' closes a map.
 * This should be called when the map is no longer required.
 * Once called, no operations are allowed on the map other than another open.
 * The result of a close is positive if the close succeded and zero if it
 * did not.
 */
int
pe_as_lit_kw_map_wr_close(struct pe_as_lit_kw_map_wr *t)
{
	size_t n_written;
	size_t n_kws;
	int err = 0;

	if (fseek(t->sink, 0L, SEEK_SET) != 0) {
		err = errno;
		goto could_not_find_file_start;
	}

	n_written = fwrite(&t->data_size, sizeof(unsigned long), 1, t->sink);
	if (n_written != 1){
		err = errno;
		goto could_not_write_size;
	}
	
	n_kws = pe_as_kw2size_t(t->map.kw_code);
	n_written = fwrite(&n_kws, sizeof(size_t), 1, t->sink);
	if (n_written != 1) {
		err = errno;
		goto could_not_write_id_code;
	}

	pe_as_lit_kw_map_close(&t->map, 0);
	if (fclose(t->sink) != 0) {
		err = errno;
		goto could_not_close_file;
	}
	return err;
    
could_not_write_size:
could_not_write_id_code:
could_not_find_file_start:
	(void)pe_as_lit_kw_map_close(&t->map, pe_as_lit_kw_map_wr_delete_strings);
	(void)fclose(t->sink);
could_not_close_file:
	return err;
}
