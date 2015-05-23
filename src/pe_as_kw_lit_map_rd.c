/*
 * Copyright (c) 1994-2015 Nineties Retro
 */

#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "pe_as_kw.h"
#include "pe_as_kw_lit_map_rd.h"

static void
pe_as_kw_lit_map_rd_init(pe_as_kw_lit_map_rd *m)
{
	size_t           i;
	char        * data = m->raw_kws;
	char const ** kws =  m->kws;

	for (i = 1; i <= m->n_kws; i += 1, data += 2+(size_t)(unsigned char)data[0]) {
		kws[i]= data;
	}
}

/*
 * Some of the awkward +1 and +2 are due to the fact that the first
 * character of an id stores its length, and that the id is also
 * null terminated.
 *
 * Some of the other awkward +1s are due to the fact id codes start
 * at 1, so to avoid any lookups having to do a -1, the first slot
 * of the kws is left blank.
 */
static int
pe_as_kw_lit_map_rd_read(pe_as_kw_lit_map_rd *m, char const *file_name, FILE *source)
{
	unsigned long data_size;
	int err = 0;

	if (fread(&data_size, sizeof(unsigned long), 1, source) != 1)
		return ferror(source);

	if (data_size == 0) {
		m->kws =     (char const **)0;
		m->raw_kws = (char *)0;
		m->n_kws =   0;
		return 0;
	}
	if (fread(&m->n_kws, sizeof(size_t), 1, source) != 1) {
		err = ferror(source);
		return err;
	}
	m->kws= (char const **)malloc((m->n_kws+1) * sizeof(char *));
	if (m->kws == (char const **)0) {
		err = ENOMEM;
		goto cannot_allocate_kws;
	}
	m->raw_kws= (char *)malloc((size_t)data_size);
	if (m->raw_kws == (char *)0) {
		err = ENOMEM;
		goto cannot_allocate_data;
	}

	if (fread(m->raw_kws, (size_t)data_size, 1, source) != 1) {
		err = ferror(source);
		goto cannot_read_data;
	}

	return err;

cannot_read_data:
	free(m->raw_kws);
cannot_allocate_data:
	free(m->kws);
cannot_allocate_kws:
	return err;
}



int
pe_as_kw_lit_map_rd_open(pe_as_kw_lit_map_rd *m, char const *file_name)
{
	FILE *source;
	int err = 0;

	source = fopen(file_name, "rb");
	if (source == 0) {
		err = errno;
		goto could_not_open_file;
	}
	err = pe_as_kw_lit_map_rd_read(m, file_name, source);
	if (err)
		goto could_not_read_kws;
	pe_as_kw_lit_map_rd_init(m);
	if (fclose(source) != 0)
		err = errno;
	return err;

could_not_read_kws:
	(void)fclose(source);
could_not_open_file:
	return err;
}



char const *
pe_as_kw_lit_map_rd_kw(const pe_as_kw_lit_map_rd *m, pe_as_kw kw, size_t *len)
{
	char const * lit;
	size_t       idx = pe_as_kw2size_t(kw);

	/*
	 * The writer ensures that the identifiers are long-word aligned
	 * so although the following is widening the alignment requirements
	 * those requirements are guaranteed to have been met.
	 */
	lit = m->kws[idx];
	*len = (size_t)(unsigned char)lit[0];
	return &lit[1];
}



size_t
pe_as_kw_lit_map_rd_n_kws(const struct pe_as_kw_lit_map_rd *m)
{
	return m->n_kws;
}



void
pe_as_kw_lit_map_rd_close(pe_as_kw_lit_map_rd *m)
{
	free(m->raw_kws);
	free(m->kws);
}
