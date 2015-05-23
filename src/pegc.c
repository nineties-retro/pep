/*
 * Copyright (c) 1994-2015 Nineties Retro
 *
 *
 * Compile a grammar.
 * To do: ensure that the grammar :-
 * - is a partial order with the start node as bottom.
 * - does not contain any (in)direct left recursion
 * - ensure that (list ... ) (list ...) does not occur (in)directly
 * - probably lots of other checks ...
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "pe_as_rd.h"
#include "pe_kw.h"
#include "pe_hash.h"
#include "pe_as_lit_kw_map_wr.h"

static char const *program_name;
static char const *grammar_prefix;

static void
usage(void)
{
	fprintf(stderr, "%s: grammar", program_name);
}


static int
process_arguments(int argc, char **argv)
{
	program_name = argv[0];
	while (++argv, --argc != 0 && argv[0][0] == '-' && argv[0][2] == '\0') {
		switch(argv[0][1]) {
		case 'h':
		default:
			usage();
			return EINVAL;
		}
	}
	if (argc != 1)  {
		usage();
		return EINVAL;
	}
	grammar_prefix = argv[0];
	return 0;
}


static int
output_keyword(struct pe_as_lit_kw_map_wr *keywords, size_t len,
	       char const *lit, pe_as_kw *kw)
{
	pe_hash h = pe_hash_lit(len, lit);
	const int fix = (lit[0] == '&') ? 1 : 0;

	return pe_as_lit_kw_map_wr_kw(keywords, len, lit, fix, h, kw);
}



static int
collect_rule_defs(struct pe_as_rd *grammar)
{
	char ascii_keyword_file_name[PATH_MAX];
	char keyword_writer_file_name[PATH_MAX];
	FILE * ascii_keyword_file;
	struct pe_as_lit_kw_map_wr keyword_writer;
	int err;
	pe_as_idx root = pe_as_rd_root(grammar);
	pe_as_idx identifier = pe_as_rd_child(grammar, root);
	pe_as_idx string = pe_as_rd_next(grammar, identifier);
	pe_as_idx integer = pe_as_rd_next(grammar, string);
	pe_as_idx start = pe_as_rd_next(grammar, integer);
	pe_as_idx p = pe_as_rd_next(grammar, start);

	err = snprintf(keyword_writer_file_name, sizeof keyword_writer_file_name, "%skw", grammar_prefix);
	if ((err < 0) || (err >= sizeof keyword_writer_file_name)) {
		err = ENAMETOOLONG;
		goto could_not_create_keyword_writer_file_name;
	}
	err = pe_as_lit_kw_map_wr_open(&keyword_writer, keyword_writer_file_name);
	if (err)
		goto could_not_open_keyword_writer;
	err = snprintf(ascii_keyword_file_name, sizeof ascii_keyword_file_name, "%skws", grammar_prefix);
	if ((err < 0) || (err >= sizeof ascii_keyword_file_name)) {
		err = ENAMETOOLONG;
		goto could_not_create_ascii_keyword_file_name;
	}
	ascii_keyword_file = fopen(ascii_keyword_file_name, "w");
	if (ascii_keyword_file == 0) {
		err = errno;
		goto could_not_open_ascii_keyword_file;
	}
	for ( ; pe_as_idx_valid(p); p = pe_as_rd_next(grammar, p)) {
		switch (pe_as_kw2size_t(pe_as_rd_kw(grammar, p))) {
		case pe_kw_keyword:
		{
			pe_as_idx rule_name = pe_as_rd_child(grammar, p);
			pe_as_str name = pe_as_rd_str(grammar, pe_as_rd_next(grammar, rule_name));
			pe_as_kw kw_;
			size_t len;
			char const *lit = pe_as_rd_str_lit_len(grammar, name, &len);
			
			err = output_keyword(&keyword_writer, len, lit, &kw_);
			if (err)
				goto could_not_output_keyword;

			if (fprintf(ascii_keyword_file, "%s\n", lit) < 0){
				err = errno;
				goto could_not_output_ascii_keyword;
			}
		}
		default:
			break;
		}
	}

	if (fclose(ascii_keyword_file) != 0) {
		err = errno;
		goto could_not_close_ascii_keyword_file;
	}
	return pe_as_lit_kw_map_wr_close(&keyword_writer);

could_not_output_keyword:
could_not_output_ascii_keyword:
	(void)fclose(ascii_keyword_file);
could_not_close_ascii_keyword_file:
	remove(ascii_keyword_file_name);
could_not_open_ascii_keyword_file:
could_not_create_ascii_keyword_file_name:
	(void)pe_as_lit_kw_map_wr_close(&keyword_writer);
	remove(keyword_writer_file_name);
could_not_open_keyword_writer:
could_not_create_keyword_writer_file_name:
	return 0;
}


int
main(int argc, char ** argv)
{
	struct pe_as_rd grammar;
	int err;

	err = process_arguments(argc, argv);
	if (err)
		goto could_not_process_arguments;
	pe_as_rd_open(&grammar);
	err = pe_as_rd_data(&grammar, grammar_prefix);
	if (err)
		goto could_not_read_grammar;
	err = collect_rule_defs(&grammar);
	if (err)
		goto could_not_collect_rule_defs;
	pe_as_rd_close(&grammar);
	return EXIT_SUCCESS;

could_not_collect_rule_defs:
could_not_read_grammar:
	(void)pe_as_rd_close(&grammar);
could_not_process_arguments:
	return err;
}

