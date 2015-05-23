/*
 * Copyright (c) 1994-2015 Nineties Retro
 */

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "pe_as_id.h"
#include "pe_as_id_lit_map_rd.h"


static void
pe_as_id_lit_map_rd_init(struct pe_as_id_lit_map_rd *m)
{
	size_t i;
	char *data = m->raw_ids;
	char const **ids =  m->ids;
	
	for (i = 1; i <= m->n_ids; i += 1, data += 2+(size_t)(unsigned char)data[0])
		ids[i]= data;
}



/*
 * Some of the awkward +1 and +2 are due to the fact that the first
 * character of an id stores its length, and that the id is also
 * null terminated.
 *
 * Some of the other awkward +1s are due to the fact id codes start
 * at 1, so to avoid any lookups having to do a -1, the first slot
 * of the ids is left blank.
 */
static int
pe_as_id_lit_map_rd_read(struct pe_as_id_lit_map_rd *m, char const *file_name, FILE *source)
{
	unsigned long data_size;
	int err = 0;

	if (fread(&data_size, sizeof(unsigned long), 1, source) != 1)
		return errno;
	if (data_size == 0) {
		m->ids = 0;
		m->raw_ids = 0;
		m->n_ids = 0;
		return 0;
	}
	if (fread(&m->n_ids, sizeof(size_t), 1, source) != 1)
		return ferror(source);
	m->ids= (char const **)malloc((m->n_ids+1) * sizeof(char *));
	if (m->ids == 0) {
		err = ENOMEM;
		goto cannot_allocate_ids;
	}
	m->raw_ids = (char *)malloc((size_t)data_size);
	if (m->raw_ids == 0) {
		err = ENOMEM;
		goto cannot_allocate_data;
	}
	if (fread(m->raw_ids, (size_t)data_size, 1, source) != 1) {
		err = errno;
		goto cannot_read_data;
	}
	return err;

cannot_read_data:
	free(m->raw_ids);
cannot_allocate_data:
	free(m->ids);
cannot_allocate_ids:
	return err;
}



int
pe_as_id_lit_map_rd_open(struct pe_as_id_lit_map_rd *m, char const *file_name)
{
	FILE *source;
	int err;

	source = fopen(file_name, "rb");
	if (source == 0) {
		err = errno;
		goto could_not_open_file;
	}
	err = pe_as_id_lit_map_rd_read(m, file_name, source);
	if (err)
		goto could_not_read_ids;
	pe_as_id_lit_map_rd_init(m);
	if (fclose(source) != 0) {
		err = errno;
		pe_as_id_lit_map_rd_close(m);
	}
	return err;

could_not_read_ids:
	(void)fclose(source);
could_not_open_file:
	return err;
}



char const *
pe_as_id_lit_map_rd_id(const struct pe_as_id_lit_map_rd *m, pe_as_id id, size_t *len)
{
	char const *lit;
	size_t idx = pe_as_id2size_t(id);

	assert(!pe_as_id_eq(id, pe_as_id_bottom()));
	assert(idx <= m->n_ids);
	/*
	 * The writer ensures that the identifiers are long-word
	 * aligned so although the following is widening the alignment
	 * requirements those requirements are guaranteed to have been
	 * met.
	 */
	lit = m->ids[idx];
	*len = (size_t)(unsigned char)lit[0];
	return &lit[1];
}


size_t
pe_as_id_lit_map_rd_n_ids(const struct pe_as_id_lit_map_rd *m)
{
	return m->n_ids;
}


void
pe_as_id_lit_map_rd_close(struct pe_as_id_lit_map_rd *m)
{
	free(m->raw_ids);
	free(m->ids);
}

