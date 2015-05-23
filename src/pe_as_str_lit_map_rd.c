/*
 * Copyright (c) 1994-2015 Nineties Retro
 */

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "pe_as_str.h"
#include "pe_as_str_lit_map_rd.h"

#define pe_as_lit_str_map_rd_str_offset sizeof(long)

int
pe_as_str_lit_map_rd_read(struct pe_as_str_lit_map_rd *m, FILE *source)
{
	int err = 0;

	if (fread(&m->total_size, sizeof(size_t), 1, source) != 1)
		return errno;
	if (fread(&m->n_strs, sizeof(size_t), 1, source) != 1)
		return ferror(source);
	if (m->n_strs == 0) {
		m->strs = 0;
	} else {
		m->strs = malloc(m->total_size+pe_as_lit_str_map_rd_str_offset);
		if (m->strs == 0) {
			err = ENOMEM;
			goto could_not_create_string_buffer;
		}
		if (fread(m->strs+pe_as_lit_str_map_rd_str_offset, m->total_size, 1, source) != 1) {
			err = ferror(source);
			goto could_not_read_strings;
		}
	}
	return err;

could_not_read_strings:
	free(m->strs);
could_not_create_string_buffer:
	return err;
}


int
pe_as_str_lit_map_rd_open(struct pe_as_str_lit_map_rd *m, char const *file_name)
{
	FILE *source;
	int err = 0;

	source = fopen(file_name, "rb");
	if (source == 0) {
		err = errno;
		goto could_not_open_string_file;
	}

	err = pe_as_str_lit_map_rd_read(m, source);
	if (err)
		goto could_not_read_strings;
	if (fclose(source) != 0)
		err = errno;
	return err;
 could_not_read_strings:
	(void)fclose(source);
could_not_open_string_file:
	return err;
}


char const *
pe_as_str_lit_map_rd_str(const struct pe_as_str_lit_map_rd *m, pe_as_str str, size_t *len)
{
	size_t i = pe_as_str2size_t(str);

	assert(i < m->total_size);
	*len = *(size_t *)(m->strs + i);
	return &m->strs[i+sizeof(pe_as_str)];
}


size_t
pe_as_str_lit_map_rd_n_strs(const struct pe_as_str_lit_map_rd *m)
{
	return m->n_strs;
}


size_t
pe_as_str_lit_map_rd_lit_space(struct pe_as_str_lit_map_rd *m)
{
	return m->total_size - m->n_strs * sizeof(size_t);
}


void
pe_as_str_lit_map_rd_close(struct pe_as_str_lit_map_rd *m)
{
	free(m->strs);
}

