/*
 * Copyright (c) 1994-2015 Nineties Retro
 */

#include <errno.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include "pe_as_rd.h"
#include "pe_as_rd_pp.h"

static int
pe_as_rd_pp_out_str(struct pe_as_rd_pp *pp, char const *str)
{
	if (fputs(str, pp->output) != EOF)
		return errno;
	return 0;
}

static int
pe_as_rd_pp_out_char(struct pe_as_rd_pp *pp, char c)
{
	if (fputc(c, pp->output) != EOF)
		return errno;
	return 0;
}


static int
pe_as_rd_pp_display_identifier(struct pe_as_rd_pp *pp, pe_as_id id)
{
	char const *lit;
	int err;

	lit = pe_as_rd_id_lit(pp->reader, id);
	err = pe_as_rd_pp_out_char(pp, ' ');
	if (err)
		return err;
	err = pe_as_rd_pp_out_str(pp, lit);
	if (err)
		return err;
	return err;
}


static int
pe_as_rd_pp_display_string(struct pe_as_rd_pp *pp, pe_as_str s)
{
	char const *lit;
	int err;

	lit = pe_as_rd_str_lit(pp->reader, s);
	err = pe_as_rd_pp_out_str(pp, " \"");
	if (err)
		return err;
	err = pe_as_rd_pp_out_str(pp, lit);
	if (err)
		return err;
	err = pe_as_rd_pp_out_char(pp, '"');
	if (err)
		return err;
	return err;
}


static int
pe_as_rd_pp_display_integer(struct pe_as_rd_pp *pp, pe_as_int i)
{
	if (fprintf(pp->output, " %ld", pe_as_int2long(i)) < 0)
		return errno;
	return 0;
}


static int
pe_as_rd_pp_display_keyword(struct pe_as_rd_pp *pp, pe_as_kw k)
{
	char const *lit;
	int err;

	err = pe_as_rd_pp_out_char(pp, '(');
	if (err)
		return err;
	lit = pe_as_rd_kw_lit(pp->reader, k);
	return pe_as_rd_pp_out_str(pp, lit);
}


static int
pe_as_rd_pp_display_indentation(struct pe_as_rd_pp *pp, size_t n)
{
	int err;

	for ( ; n != 0; n--) {
		err = pe_as_rd_pp_out_char(pp, ' ');
		if (err)
			return err;
	}
	return 0;
}


static int
pe_as_rd_pp_keyword(struct pe_as_rd_pp *pp, pe_as_idx i, size_t indentation)
{
	struct pe_as_rd *r = pp->reader;
	int err;

	if (indentation != 0) {
		err = pe_as_rd_pp_out_char(pp, '\n');
		if (err)
			return err;
	}
	err = pe_as_rd_pp_display_indentation(pp, indentation);
	if (err)
		return err;
	err = pe_as_rd_pp_display_keyword(pp, pe_as_rd_kw(r, i));
	if (err)
		return err;
	i = pe_as_rd_child(r, i);
	for (; pe_as_idx_valid(i); i = pe_as_rd_next(r, i)) {
		switch (pe_as_rd_type(r, i)) {
		case pe_as_nt_kw:
			err = pe_as_rd_pp_keyword(pp, i, indentation+pp->indentation);
			if (err)
				return err;
			break;
		case pe_as_nt_id:
			err = pe_as_rd_pp_display_identifier(pp, pe_as_rd_id(r, i));
			if (err)
				return err;
			break;
		case pe_as_nt_int:
			err = pe_as_rd_pp_display_integer(pp, pe_as_rd_int(r, i));
			if (err)
				return err;
			break;
		case pe_as_nt_str:
			err = pe_as_rd_pp_display_string(pp, pe_as_rd_str(r, i));
			if (err)
				return err;
			break;
		}
	}
	err = pe_as_rd_pp_out_char(pp, ')');
	if (err)
		return err;
	if (indentation == 0) {
		err = pe_as_rd_pp_out_char(pp, '\n');
		if (err)
			return err;
	}
	return 0;
}


int
pe_as_rd_pp_display(struct pe_as_rd_pp *pp, pe_as_idx i)
{
	return pe_as_rd_pp_keyword(pp, i, 0);
}


void
pe_as_rd_pp_open(struct pe_as_rd_pp *pp, struct pe_as_rd *r)
{
	pp->reader = r;
	pp->indentation = 2;
	pp->output = stdout;
}


void
pe_as_rd_pp_output(struct pe_as_rd_pp *pp, FILE *output)
{
	pp->output= output;
}


void
pe_as_rd_pp_indentation(struct pe_as_rd_pp *pp, size_t indentation)
{
	pp->indentation = indentation;
}


void
pe_as_rd_pp_close(struct pe_as_rd_pp *pp)
{
}
