/*
 * Copyright (c) 1994-2015 Nineties Retro
 */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "pe_as_str.h"
#include "pe_as_lit_str_map_wr.h"

#define pe_as_lit_str_map_wr_str_offset sizeof(long)


/*
 * Watch out for the fiddly bits that ensure that each string is 
 * null terminated and that it is has enough trailing padding so
 * that the next string will be aligned on a sizeof(size_t) boundary
 * when the data is read in as a complete unit by the reader.
 */
int
pe_as_lit_str_map_wr_str(pe_as_lit_str_map_wr *m, size_t len,
			 char const *id, pe_as_str *code)
{
	size_t n_written;
	size_t slop = 0;
	int err = 0;

	n_written = fwrite(&len, sizeof(size_t), 1, m->sink);
	if (n_written != 1)
		return ferror(m->sink);
	n_written = fwrite(id, 1, len, m->sink);
	if (n_written != len)
		return ferror(m->sink);
	n_written = fwrite(&slop, 1, 1, m->sink);
	if (n_written != 1)
		return ferror(m->sink);

	if ((slop = sizeof(size_t) - ((len%sizeof(size_t))+1)) != 0) {
		n_written = fwrite(&slop, slop, 1, m->sink);
		if (n_written != 1) {
			return ferror(m->sink);
		}
	}

	*code= size_t2pe_as_str(m->total_size);
	m->total_size += len + sizeof(size_t) + slop +1;
	m->n_strs += 1;
	return err;
}



int
pe_as_lit_str_map_wr_open(pe_as_lit_str_map_wr *m, char const *file_name)
{
	size_t total_size = 0;
	size_t n_written = 0;
	int err = 0;

	if (m->sink= fopen(file_name, "w+b"), m->sink == (FILE *)0)
		return errno;

	if (fwrite(&total_size, sizeof(size_t), 1, m->sink) != 1) {
		err = ferror(m->sink);
		goto could_not_write_total_size;
	}

	if (fwrite(&n_written, sizeof(size_t), 1, m->sink) != 1) {
		err = ferror(m->sink);
		goto could_not_write_n_strs;
	}

	m->total_size= pe_as_lit_str_map_wr_str_offset;
	return err;

could_not_write_n_strs:
could_not_write_total_size:
	(void)fclose(m->sink);
	return err;
}



int
pe_as_lit_str_map_wr_close(pe_as_lit_str_map_wr *m)
{
	size_t total_size;
	size_t n_written;
	int err = 0;

	total_size= m->total_size - pe_as_lit_str_map_wr_str_offset;
	if (fseek(m->sink, 0L, SEEK_SET) != 0) {
		err = errno;
		goto could_not_seek_to_start;
	}

	n_written = fwrite(&total_size, sizeof(size_t), 1, m->sink);
	if (n_written != 1) {
		err = ferror(m->sink);
		goto could_not_write_total_size;
	}

	n_written = fwrite(&m->n_strs, sizeof(size_t), 1, m->sink);
	if (n_written != 1) {
		err = ferror(m->sink);
		goto could_not_write_n_strs;
	}

	if (fclose(m->sink) != 0) {
		err = errno;
		goto could_not_close_sink;
	}
	return err;

could_not_write_n_strs:
could_not_write_total_size:
could_not_seek_to_start:
	(void)fclose(m->sink);
could_not_close_sink:
	return err;
}
