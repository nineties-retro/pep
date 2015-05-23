/*
 * Copyright (c) 1994-2015 Nineties Retro
 */

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include "pe_as_rd.h"

int
pe_as_rd_in_bounds(const struct pe_as_rd *r, pe_as_lidx p)
{
	pe_as_lidx root = pe_as_rd_root(r);

	return pe_as_lidx_le(root, p)
		&& pe_as_lidx_le(p, ulong2pe_as_lidx(pe_as_lidx2ulong(root) + pe_as_rd_n_node(r)));
}



int
pe_as_rd_is_open(const struct pe_as_rd *r)
{
	return r != 0 && r->nodes != 0;
}



int
pe_as_rd_language_defined(const struct pe_as_rd *rd)
{
	return rd->language != (char const *)0;
}



/*
 * Returns a pointer to the node at the given `index' in the `store'.
 * Assumes that `index' is a valid index in `store'.
 */
struct pe_as_lnode *
pe_as_rd_at(const struct pe_as_rd *r, pe_as_lidx p)
{
	assert(pe_as_rd_in_bounds(r, p));

	return &r->nodes[pe_as_lidx2ulong(p)];
}



int
pe_as_rd_read(struct pe_as_rd * rd, FILE *f)
{
	int err = 0;
	size_t n_read;
	char magic[pe_as_lnode_rw_magic_size];

	if (fread(magic, sizeof magic, 1, f) != 1) {
		err = ENODATA;
		goto could_not_read_magic;
	}
	if (memcmp(magic, pe_as_lnode_rw_magic, sizeof magic) != 0) {
		err = ENOEXEC;
		goto not_a_node_file;
	}

	if (fseek(f, pe_as_lnode_rw_n_nodes_offset, SEEK_SET) != 0) {
		err = errno;
		goto could_not_seek_to_file_start;
	}

	if (fread((void *)&rd->n_nodes, sizeof(size_t), 1, f) != 1) {
		err = ENODATA;
		goto could_not_read_file_size;
	}

	rd->nodes = malloc((rd->n_nodes+1)*sizeof(struct pe_as_lnode));
	if (rd->nodes == 0) {
		err = ENOMEM;
		goto could_not_create_buffer;
	}

	if (fseek(f, pe_as_lnode_rw_data_offset, SEEK_SET) != 0) {
		err = errno;
		goto could_not_read_n_nodes;
	}
	n_read = fread((void *)&rd->nodes[1], sizeof(struct pe_as_lnode), rd->n_nodes, f);
	if (n_read != rd->n_nodes) {
		err = ENODATA;
		goto could_not_read_n_nodes;
	}
	return err;

could_not_read_n_nodes:
	free(rd->nodes);
could_not_create_buffer:
could_not_seek_to_file_start:
could_not_read_file_size:
not_a_node_file:
could_not_read_magic:
	return err;
}



/*
 * Searches for a node with a given `line_number' between the nodes `start'
 * and `end' inclusive.  If no node with the right `line_number' is found,
 * `default_pos' is returned.
 *
 * Since the file is indexed by node position not line number, finding
 * the node at a given line number requires searching.  Fortunately, 
 * since the file is ordered, it is possible to perform a binary search.
 * Could possibly use the standard C library function BSEARCH to do this.
 */
static size_t
pe_as_rd_search_for(const struct pe_as_rd *rd, size_t s, size_t e, size_t l, size_t i)
{
	size_t j, p;

	assert(s <= e);

	while (s != e) {
		j= (s+e)/2;
		p= (size_t)pe_as_line2ulong(pe_as_rd_line(rd, ulong2pe_as_lidx(j)));
		if (l < p) e= j;
		else if (p < l) s= (s==j)?j+1:j;
		else return pe_as_rd_search_for(rd, s, j, l, j);
	}
	return i;
}



pe_as_lidx
pe_as_rd_on_line(const struct pe_as_rd *rd, unsigned long l)
{
	size_t s = (size_t)pe_as_lidx2ulong(pe_as_rd_root(rd));
	size_t e = s + pe_as_rd_n_node(rd) - 1;

	assert(((unsigned long)l) <= pe_as_line2ulong(pe_as_rd_line(rd, ulong2pe_as_lidx(e))));
	return ulong2pe_as_lidx((unsigned long)pe_as_rd_search_for(rd, s, e, (size_t)l, 0));
}



pe_as_lidx
pe_as_rd_child(const struct pe_as_rd * r, pe_as_lidx p)
{
	assert(pe_as_rd_is_open(r));

	return pe_as_lnode_has_child(pe_as_rd_at(r, p)) 
		? pe_as_lidx_succ(p)
		: pe_as_lidx_bottom();
}



pe_as_id
pe_as_rd_id(const struct pe_as_rd * r, pe_as_lidx p)
{
	struct pe_as_lnode * n;

	assert(pe_as_rd_is_open(r));
	n = pe_as_rd_at(r, p);
	assert(pe_as_lnode_type(n) == pe_as_nt_id);
	return n->v.id;
}



char const *
pe_as_rd_id_lit(const struct pe_as_rd * r, pe_as_id i)
{
	size_t len;

	assert(pe_as_rd_is_open(r));
	return pe_as_id_lit_map_rd_id(&r->ids, i, &len);
}


char const *
pe_as_rd_id_lit_len(const struct pe_as_rd * r, pe_as_id i, size_t *len)
{
	assert(pe_as_rd_is_open(r));
	return pe_as_id_lit_map_rd_id(&r->ids, i, len);
}



pe_as_int
pe_as_rd_int(const struct pe_as_rd *r, pe_as_lidx p)
{
	struct pe_as_lnode * n;

	assert(pe_as_rd_is_open(r));
	n = pe_as_rd_at(r, p);
	assert(pe_as_lnode_type(n) == pe_as_nt_int);
	return n->v.i;
}



pe_as_kw
pe_as_rd_kw(const struct pe_as_rd * r, pe_as_lidx p)
{
	struct pe_as_lnode *n;

	assert(pe_as_rd_is_open(r));
	n = pe_as_rd_at(r, p);
	assert(pe_as_lnode_type(n) == pe_as_nt_kw);
	return n->v.kw;
}


char const *
pe_as_rd_kw_lit(const struct pe_as_rd * r, pe_as_kw kw)
{
	size_t len;

	assert(pe_as_rd_is_open(r));
	assert(pe_as_rd_language_defined(r));
	return pe_as_kw_lit_map_rd_kw(&r->kws, kw, &len);
}


char const *
pe_as_rd_kw_lit_len(const struct pe_as_rd *r, pe_as_kw kw, size_t *len)
{
	assert(pe_as_rd_is_open(r));
	return pe_as_kw_lit_map_rd_kw(&r->kws, kw, len);
}



pe_as_line
pe_as_rd_line(const struct pe_as_rd *r, pe_as_lidx p)
{
	struct pe_as_lnode *n;

	assert(pe_as_rd_is_open(r));
	n= pe_as_rd_at(r, p);
	return n->line;
}


size_t
pe_as_rd_n_id(const struct pe_as_rd * r)
{
	assert(pe_as_rd_is_open(r));
	return pe_as_id_lit_map_rd_n_ids(&r->ids);
}


size_t
pe_as_rd_n_kw(const struct pe_as_rd * r)
{
	assert(pe_as_rd_is_open(r));
	return pe_as_kw_lit_map_rd_n_kws(&r->kws);
}


size_t
pe_as_rd_n_node(const struct pe_as_rd *r)
{
	assert(pe_as_rd_is_open(r));
	return r->n_nodes;
}



size_t
pe_as_rd_n_str(const struct pe_as_rd * r)
{
	assert(pe_as_rd_is_open(r));
	return pe_as_str_lit_map_rd_n_strs(&r->strs);
}



pe_as_lidx
pe_as_rd_next(const struct pe_as_rd * r, pe_as_lidx p)
{
	struct pe_as_lnode *n;

	assert(pe_as_rd_is_open(r));
	n = pe_as_rd_at(r, p);
	return n->next;
}



void
pe_as_rd_close_kws(struct pe_as_rd *rd)
{
	pe_as_kw_lit_map_rd_close(&rd->kws);
}



static int
pe_as_rd_open_kws(struct pe_as_rd *rd, char const *language_prefix)
{
	char file_name[PATH_MAX];
	int err;
  
	err = snprintf(file_name, sizeof file_name, "%skw", language_prefix);
	if ((err < 0) || (err >= sizeof(file_name)))
		return ENAMETOOLONG;
	return pe_as_kw_lit_map_rd_open(&rd->kws, file_name);
}



static void
pe_as_rd_close_ids(struct pe_as_rd *rd)
{
	pe_as_id_lit_map_rd_close(&rd->ids);
}



static int
pe_as_rd_open_ids(struct pe_as_rd *rd, char const *file_name_prefix)
{
	char file_name[PATH_MAX];
	int err;
  
	err = snprintf(file_name, sizeof file_name, "%sid", file_name_prefix);
	if ((err < 0) || (err >= sizeof(file_name)))
		return ENAMETOOLONG;
	return pe_as_id_lit_map_rd_open(&rd->ids, file_name);
}



static void
pe_as_rd_close_strs(struct pe_as_rd * rd)
{
	pe_as_str_lit_map_rd_close(&rd->strs);
}




static int
pe_as_rd_open_strs(struct pe_as_rd * rd, char const *file_name_prefix)
{
	char file_name[PATH_MAX];
	int err;
  
	err = snprintf(file_name, sizeof file_name, "%sstr", file_name_prefix);
	if ((err < 0) || (err >= sizeof(file_name)))
		return ENAMETOOLONG;
	return pe_as_str_lit_map_rd_open(&rd->strs, file_name);
}




static void
pe_as_rd_close_nodes(struct pe_as_rd *rd)
{
	free(rd->nodes);
}



static int
pe_as_rd_open_nodes(struct pe_as_rd *rd, char const *file_name_prefix)
{
	FILE *ns;
	int err;
	char file_name[PATH_MAX];

	err = snprintf(file_name, sizeof file_name, "%sas", file_name_prefix);
	if ((err < 0) || (err >= sizeof(file_name)))
		return ENAMETOOLONG;
	if ((ns= fopen(file_name, "rb")) == 0) {
		err = errno;
		goto could_not_open_nodes_file;
	}
	rd->nodes= 0;
	rd->n_nodes= 0;
	err = pe_as_rd_read(rd, ns);
	if (err)
		goto could_not_read_nodes_file;
	if (fclose(ns) != 0)
		err = errno;
	return err;

could_not_read_nodes_file:
	(void)fclose(ns);
could_not_open_nodes_file:
	return err;
}



void
pe_as_rd_open(struct pe_as_rd * rd)
{
	rd->language = 0;
	rd->n_nodes = 0;
	rd->nodes = 0;
}



int
pe_as_rd_language(struct pe_as_rd *rd, char const *language_prefix)
{
	int err;

	assert(!pe_as_rd_is_open(rd));
	assert(!pe_as_rd_language_defined(rd));
	err = pe_as_rd_open_kws(rd, language_prefix);
	if (err)
		return err;
	rd->language = language_prefix;
	return 0;
}



int
pe_as_rd_data(struct pe_as_rd *rd, char const *data_prefix)
{
	int err;

	assert(!pe_as_rd_is_open(rd));
	err = pe_as_rd_open_nodes(rd, data_prefix);
	if (err)
		goto could_not_open_nodes;
	err = pe_as_rd_open_ids(rd, data_prefix);
	if (err)
		goto could_not_open_ids;
	err = pe_as_rd_open_strs(rd, data_prefix);
	if (err)
		goto could_not_open_strs;
	return err;

could_not_open_strs:
	pe_as_rd_close_ids(rd);
could_not_open_ids:
	pe_as_rd_close_nodes(rd);
could_not_open_nodes:
	return err;
}



pe_as_lidx
pe_as_rd_parent(const struct pe_as_rd *r, pe_as_lidx p)
{
	struct pe_as_lnode *n;

	assert(pe_as_rd_is_open(r));
	assert(pe_as_lidx_valid(p));
	n = pe_as_rd_at(r, p);
	return n->parent;
}



pe_as_lidx
pe_as_rd_root(const struct pe_as_rd *rd)
{
	assert(pe_as_rd_is_open(rd));
	return pe_as_lidx_succ(pe_as_lidx_bottom());
}



pe_as_str
pe_as_rd_str(const struct pe_as_rd *r, pe_as_lidx p)
{
	struct pe_as_lnode *n;

	assert(pe_as_rd_is_open(r));
	n = pe_as_rd_at(r, p);
	assert(pe_as_lnode_type(n) == pe_as_nt_str);
	return n->v.str;
}



char const *
pe_as_rd_str_lit(const struct pe_as_rd *r, pe_as_str s)
{
	size_t len;
	assert(pe_as_rd_is_open(r));
	return pe_as_str_lit_map_rd_str(&r->strs, s, &len);
}



char const *
pe_as_rd_str_lit_len(const struct pe_as_rd * r, pe_as_str s, size_t *len)
{
	assert(pe_as_rd_is_open(r));
	return pe_as_str_lit_map_rd_str(&r->strs, s, len);
}



pe_as_nt
pe_as_rd_type(const struct pe_as_rd * r, pe_as_lidx p)
{
	struct pe_as_lnode *n;

	assert(pe_as_rd_is_open(r));
	n = pe_as_rd_at(r, p);
	return pe_as_lnode_type(n);
}



void
pe_as_rd_close(struct pe_as_rd *rd)
{
	if (pe_as_rd_language_defined(rd))
		pe_as_rd_close_kws(rd);
	if (pe_as_rd_is_open(rd)) {
		pe_as_rd_close_nodes(rd);
		pe_as_rd_close_ids(rd);
		pe_as_rd_close_strs(rd);
	}
}
