/*
 * Copyright (c) 1994-2015 Nineties Retro
 */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "pe_as_id.h"
#include "pe_hash.h"
#include "pe_as_lit_id_map_wr.h"


static void
pe_as_lit_id_map_wr_free_id(char *id)
{
	free(id);
}


/*
 * The awkward +1 and +2s in the following are due to the fact that the
 * first character of an id encodes the length of the id and the fact that
 * ids are null terminated.
 */
int
pe_as_lit_id_map_wr_id(struct pe_as_lit_id_map_wr *m, size_t len, char const *id, int fix, pe_hash hash, pe_as_id *code)
{
	struct pe_as_lit_id_map_node ** n, *l;

	if (len >= 256)
		return ENAMETOOLONG;
	n = pe_as_lit_id_map_find(&m->map, len, id, fix, hash);
	l = *n;
	if (l == 0) {
		size_t n_written;
		int err = 0;
		char *lit = malloc(len+2);
		
		if (lit == 0) {
			err = ENOMEM;
			goto could_not_create_node;
		}
		lit[0]= (unsigned char)len;
		memcpy(&lit[1], id, len);
		lit[len+1]= '\0';

		err = pe_as_lit_id_map_enter(&m->map, len, lit, fix, n);
		if (err)
			goto could_not_add_node;

		if ((n_written = fwrite(lit, len+2, 1, m->sink)) != 1) {
			err = errno;
			goto could_not_write_node;
		}
		m->data_size += len+2;
		*code= (*n)->code= m->map.id_code;
		return err;

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



int
pe_as_lit_id_map_wr_open(struct pe_as_lit_id_map_wr *m, char const *file_name)
{
	size_t n_written;
	int err;

	err = pe_as_lit_id_map_open(&m->map);
	if (err)
		return err;
	if ((m->sink = fopen(file_name, "w+b")) == (FILE *)0) {
		err = errno;
		goto could_not_open_file;
	}
	m->data_size= 0;
	n_written = fwrite(&m->data_size, sizeof(unsigned long), 1 ,m->sink);
	if (n_written != 1) {
		err = errno;
		goto could_not_write_size;
	}

	n_written = fwrite(&m->map.id_code, sizeof(pe_as_id), 1, m->sink);
	if (n_written != 1) {
		err = errno;
		goto could_not_write_id_code;
	}

	return err;

could_not_write_id_code:
could_not_write_size:
	(void)fclose(m->sink);
could_not_open_file:
	return 0;
}


int
pe_as_lit_id_map_wr_close(struct pe_as_lit_id_map_wr *t)
{
	FILE *sink = t->sink;
	size_t n_written;
	size_t n_ids;
	int err = 0;

	if (fseek(sink, 0L, SEEK_SET) != 0) {
		err = errno;
		goto could_not_find_file_start;
	}
	n_written = fwrite(&t->data_size, sizeof(unsigned long), 1, sink);
	if (n_written != 1) {
		err = errno;
		goto could_not_write_size;
	}
	n_ids = pe_as_id_code(t->map.id_code);
	n_written = fwrite(&n_ids, sizeof(size_t), 1, sink);
	if (n_written != 1) {
		err = errno;
		goto could_not_write_id_code;
	}
	pe_as_lit_id_map_close(&t->map, pe_as_lit_id_map_wr_free_id);
	if (fclose(sink) != 0) {
		err = errno;
		goto could_not_close_file;
	}

	return err;
    
could_not_write_size:
could_not_write_id_code:
could_not_find_file_start:
	pe_as_lit_id_map_close(&t->map, pe_as_lit_id_map_wr_free_id);
	(void)fclose(sink);
could_not_close_file:
	return err;
}
