/*
 * Copyright (c) 1994-2015 Nineties Retro
 *
 * Generates a "kw" file holding the names of all the keywords in the
 * PEG.  This is used as part of the boot process for PEP.
 */

#include <limits.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "pe_hash.h"
#include "pe_as_kw.h"
#include "pe_as_lit_kw_map_wr.h"

static char const *program_name;
static char const *output_prefix= "./";

static char const *
keywords[]= {
	"alias",
	"comment",
	"grammar",
	"identifier",
	"integer",
	"keyword",
	"list",
	"optional",
	"or",
	"start",
	"string",
	"unique",
	0
};


static void usage(void)
{
	fprintf(stderr, "%s: [ -o <prefix> ]\n", program_name);
}


static char const *
string_arg(int * argc, char ***argv)
{
	if (*argc < 2) {
		usage();
		return 0;
	}
	*argc -= 1;  *argv += 1;
	return (*argv)[0];
}


static int
process_arguments(int argc, char **argv)
{
	program_name = argv[0];
	while (++argv, --argc != 0 && argv[0][0] == '-'&& argv[0][2] == '\0') {
		switch(argv[0][1]) {
		case 'o':
			if ((output_prefix = string_arg(&argc, &argv)) == 0)
				return 0;
			break;
		default:
			usage();
			return 0;
		}
	}
	if (argc != 0) {
		usage();
		return 0;
	}
	return 1;
}



int main(int argc, char ** argv)
{
	char const **k;
	int n;
	struct pe_as_lit_kw_map_wr out;
	char file_name[PATH_MAX];
	
	if (!process_arguments(argc, argv))
		goto could_not_process_arguments;

	n = snprintf(file_name, sizeof file_name, "%skw", output_prefix);
	if ((n < 0) || (n >= sizeof file_name))
		goto could_not_create_keyword_file_name;

	if (pe_as_lit_kw_map_wr_open(&out, file_name))
		goto could_not_open_keyword_file;

	for (k = &keywords[0]; *k != 0; k += 1) {
		pe_as_kw code;
		size_t len = strlen(*k);
		pe_hash h = pe_hash_lit(len, *k);
		if (pe_as_lit_kw_map_wr_kw(&out, len, *k, 0, h, &code))
			goto could_not_write_keyword;
	}

	if (pe_as_lit_kw_map_wr_close(&out))
		goto could_not_close_keyword_file;
	return EXIT_SUCCESS;

could_not_write_keyword:
	(void)pe_as_lit_kw_map_wr_close(&out);
could_not_close_keyword_file:
could_not_open_keyword_file:
could_not_create_keyword_file_name:
could_not_process_arguments:
	return EXIT_FAILURE;
}
