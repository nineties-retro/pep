/*
 * Copyright (c) 1994-2015 Nineties Retro
 */

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "pe_as_id.h"
#include "pe_as_kw.h"
#include "pe_as_str.h"
#include "pe_as_int.h"
#include "pe_as_line.h"
#include "pe_as_lwr.h"


static struct pe_as_lwr_idx *
pe_as_lwr_idx_map_at(struct pe_as_lwr_idx_map *m, size_t idx)
{
	assert(idx < m->size);
	return &m->buffer[idx];
}


static int
pe_as_lwr_idx_map_at_put(struct pe_as_lwr_idx_map *m, size_t idx, pe_as_lidx pos, int on_disk)
{
	if (idx >= m->size) {
		size_t size= idx * 2;
		struct pe_as_lwr_idx *b = realloc(m->buffer, size * sizeof(struct pe_as_lwr_idx));
		if (b == 0)
			return ENOMEM;
		m->size= size;
		m->buffer = b;
	}
	m->buffer[idx].p = pos;
	m->buffer[idx].on_disk = on_disk;
	return 0;
}



static void
pe_as_lwr_idx_map_close(struct pe_as_lwr_idx_map *m) 
{
	free(m->buffer);
	m->buffer = 0;
	m->size = 0;
}



static int
pe_as_lwr_idx_map_open(struct pe_as_lwr_idx_map *m, size_t initial_size)
{
	m->buffer = malloc(initial_size * sizeof(struct pe_as_lwr_idx));
	if (m->buffer == 0)
		return ENOMEM;
	m->size = initial_size;
	return 0;
}


static int
pe_as_lwr_size(struct pe_as_lwr *out)
{
	FILE *sink = out->sink;

	if (fseek(sink, pe_as_lnode_rw_n_nodes_offset, SEEK_SET) != 0)
		return errno;
	if (fwrite((const void *)&out->n_nodes, sizeof(size_t), 1, sink) != 1)
		return errno;
	return 0;
}



static int
pe_as_lwr_flush(struct pe_as_lwr *out)
{
	FILE *const file = out->sink;
	const size_t bs = out->buffer_size;
	const size_t bss = out->buffer_segment_size;
	size_t ns = out->next_segment;
	size_t n_left = out->n_nodes - ns;
	struct pe_as_lnode *const buffer = out->buffer;
  
	while (n_left != 0) {
		size_t n_out = (n_left < bss) ? n_left : bss;
		struct pe_as_lnode *xxx = buffer + (ns % bs);
		if (fwrite(xxx, sizeof *xxx, n_out, file) != n_out)
			return errno;
		n_left -= n_out;
		ns += bss;
	}
	return 0;
}



static int
pe_as_lwr_write_header(struct pe_as_lwr *out)
{
	FILE *const sink= out->sink;
  
	if (fseek(sink, pe_as_lnode_rw_magic_offset, SEEK_SET) != 0)
		return errno;
	if (fwrite((const void *) pe_as_lnode_rw_magic, pe_as_lnode_rw_magic_size, 1, sink) != 1)
		return errno;
	return pe_as_lwr_size(out);
}


int
pe_as_lwr_close(struct pe_as_lwr *out)
{
	int err = 0;
  
	err = pe_as_lwr_flush(out);
	if (err)
		goto could_not_flush_nodes;
	err = pe_as_lwr_size(out);
	if (err)
		goto could_not_write_size;
	pe_as_lwr_idx_map_close(&out->last_at_level);
	free(out->buffer);
	if (fclose(out->sink) != 0) {
		err = errno;
		goto could_not_close_output;
	}
	return err;

could_not_flush_nodes:
could_not_write_size:
	(void)pe_as_lwr_idx_map_close(&out->last_at_level);
	free(out->buffer);
	(void)fclose(out->sink);
could_not_close_output:
	return err;
}



int
pe_as_lwr_open(struct pe_as_lwr *out, char const *file_name)
{
	int err = 0;

	out->sink = fopen(file_name, "w+b");
	if (out->sink == (FILE *)0) {
		err = errno;
		goto could_not_open_output;
	}

	out->nesting_level = 0;
	out->next_segment = 0;

	err = pe_as_lwr_idx_map_open(&out->last_at_level, 20);
	if (err)
		goto could_not_open_idx_map;

	out->buffer = (struct pe_as_lnode *)malloc((size_t)(out->buffer_size * sizeof(struct pe_as_lnode)));
	if (out->buffer == 0) {
		err = ENOMEM;
		goto could_not_create_buffer;
	}

	err = pe_as_lwr_idx_map_at_put(&out->last_at_level, 0, pe_as_lidx_bottom(),0);
	if (err)
		goto could_not_insert;

	out->n_nodes = 0;
	err = pe_as_lwr_write_header(out);
	if (err)
		goto could_not_write_header;
	return err;

could_not_write_header:
could_not_insert:
	(void)pe_as_lwr_idx_map_close(&out->last_at_level);
could_not_create_buffer:
	free(out->buffer);
could_not_open_idx_map:
	(void)fclose(out->sink);
	(void)remove(file_name);
could_not_open_output:
	return err;
}


static int
pe_as_lwr_patch_next_on_disk(struct pe_as_lwr *out, pe_as_lidx pos,
			     pe_as_lidx next_pos)
{
	FILE * const file = out->sink;
	long node_offset = (long)((pe_as_lidx2ulong(pos) * sizeof(struct pe_as_lnode))+pe_as_lnode_rw_data_offset);
	long segment_offset = (long)((out->next_segment * sizeof(struct pe_as_lnode)) + pe_as_lnode_rw_data_offset);
	struct pe_as_lnode n;

	if (fseek(file, node_offset, SEEK_SET) != 0)
		return errno;
	if (fread((void *)&n, sizeof n, 1, file) != 1)
		return errno;
	if (fseek(file, node_offset,  SEEK_SET) != 0)
		return errno;
	n.next= next_pos;
	if (fwrite((const void *)&n, sizeof n, 1, file) != 1)
		return errno;
	if (fseek(file, segment_offset, SEEK_SET) != 0)
		return errno;
	return 0;
}


static int
pe_as_lwr_patch_parent_on_disk(struct pe_as_lwr *out, pe_as_lidx pos)
{
	FILE *const file = out->sink;
	struct pe_as_lnode n;
	const long node_offset = (long)((pe_as_lidx2ulong(pos) * sizeof(n)) + pe_as_lnode_rw_data_offset);
	const long segment_offset = (long)((out->next_segment * sizeof(n)) + pe_as_lnode_rw_data_offset);
	
	if (fseek(file, node_offset, SEEK_SET) != 0)
		return errno;
	if (fread((void *)&n, sizeof n, 1, file) != 1)
		return errno;
	if (fseek(file, node_offset, SEEK_SET) != 0)
		return errno;
	pe_as_lnode_make_parent(&n);
	if (fwrite((void *)&n, sizeof n, 1, file) != 1)
		return errno;
	if (fseek(file, segment_offset, SEEK_SET) != 0)
		return errno;
	return 0;
}



static int
pe_as_lwr_in_written_segment(struct pe_as_lwr *out, pe_as_lidx index)
{
	const size_t next_segment = out->next_segment;
	const size_t buffer_segment_size = out->buffer_segment_size;
	const unsigned long p = pe_as_lidx2ulong(index);
	return p >= next_segment && p < next_segment + buffer_segment_size;
}



/*
 * Update the positions buffer so that any level whose last
 * token has now been written out to disk is now marked as such.
 */
static void
pe_as_lwr_update_level_map(struct pe_as_lwr *out, size_t level)
{
	struct pe_as_lwr_idx_map *const last_at_level = &out->last_at_level;
	size_t l= 0;

	for (; l < level; l++) {
		struct pe_as_lwr_idx *lp = pe_as_lwr_idx_map_at(last_at_level, l);

		if (pe_as_lwr_in_written_segment(out, lp->p))
			lp->on_disk= !0;
	}
}



static int
pe_as_lwr_node(struct pe_as_lwr *out)
{
	const size_t bs = out->buffer_size;
	const pe_as_lidx pos = ulong2pe_as_lidx(out->n_nodes);
	const pe_as_lidx next_pos = pe_as_lidx_succ(pos);
	struct pe_as_lnode *const buffer = out->buffer;
	const size_t level = out->nesting_level;
	struct pe_as_lnode *t = &out->node;
	int err;

	{
		const struct pe_as_lwr_idx *elder_sibling = 
			pe_as_lwr_idx_map_at(&out->last_at_level, level);
		if (pe_as_lidx_valid(elder_sibling->p)) {
			const pe_as_lidx logical_pos = next_pos;
			if (elder_sibling->on_disk) {
				err = pe_as_lwr_patch_next_on_disk(out, elder_sibling->p, logical_pos);
				if (err)
					return err;
			} else {
				buffer[pe_as_lidx2ulong(elder_sibling->p)%bs].next= logical_pos;
			}
		}
	}

	{ 
		err = pe_as_lwr_idx_map_at_put(&out->last_at_level, level, pos, 0);
		if (err)
			return err;
	}

	{
		pe_as_lidx lp;
		if (level != 0) {
			struct pe_as_lwr_idx *pp = pe_as_lwr_idx_map_at(&out->last_at_level, level - 1);
			lp = pe_as_lidx_succ(pp->p);
			if (pp->on_disk) {
				err = pe_as_lwr_patch_parent_on_disk(out, pp->p);
				if (err)
					return err;
			} else {
				pe_as_lnode_make_parent(&buffer[pe_as_lidx2ulong(pp->p)%bs]);
			}
		} else {
			lp = pe_as_lidx_bottom();
		}
		t->parent = lp;
	}

	{
		/*
		 * Add a token to the token file in the given
		 * position.  If adding this token moves you into a
		 * new segment, then first flush out all the items in
		 * the segment to the token file then add the item to
		 * the start of the flushed segment.
		 */
		const size_t bss = out->buffer_segment_size;
		size_t ns = out->next_segment;
		
		if (pe_as_lidx_valid(pos)
		    &&  pe_as_lidx2ulong(pos) % bs == ns % bs) {
			if (fwrite((const void *)(buffer + ns%bs), sizeof *t, bss, out->sink) != bss)
				return errno;
			pe_as_lwr_update_level_map(out, level);
			ns += bss;
		}
		out->next_segment = ns;
	}

	{
		struct pe_as_lnode *n = &buffer[pe_as_lidx2ulong(pos)%bs];
		n->tag = t->tag;
		n->line = t->line;
		n->parent = t->parent;
		n->next = pe_as_lidx_bottom();
		n->v = t->v;
	}
	out->n_nodes = pe_as_lidx2ulong(next_pos);
	return 0;
}



int
pe_as_lwr_eos(struct pe_as_lwr *out)
{
	if (out->nesting_level == 0)
		return EINVAL;
	out->nesting_level -= 1;
	return 0;
}



int
pe_as_lwr_id(struct pe_as_lwr *out, pe_as_id id)
{
	out->node.tag = pe_as_nt_id;
	out->node.v.id = id;
	return pe_as_lwr_node(out);
}



int
pe_as_lwr_int(struct pe_as_lwr *out, pe_as_int i)
{
	out->node.tag = pe_as_nt_int;
	out->node.v.i = i;
	return pe_as_lwr_node(out);
}



int
pe_as_lwr_kw(struct pe_as_lwr *out, pe_as_kw kw)
{
	int err;

	out->node.tag = pe_as_nt_kw;
	out->node.v.kw = kw;
	err = pe_as_lwr_node(out);
	if (err)
		return err;
	return pe_as_lwr_idx_map_at_put(&out->last_at_level, ++out->nesting_level, pe_as_lidx_bottom(), 0);
}



void
pe_as_lwr_line(struct pe_as_lwr *out, pe_as_line line)
{
	out->node.line = line;
}



int
pe_as_lwr_str(struct pe_as_lwr *out, pe_as_str str)
{
	out->node.tag = pe_as_nt_str;
	out->node.v.str = str;
	return pe_as_lwr_node(out);
}



void
pe_as_lwr_set_buffers(struct pe_as_lwr *out, size_t n_buffers, size_t buffer_size)
{
	out->buffer_size = buffer_size*n_buffers;
	out->buffer_segment_size = buffer_size;
}
