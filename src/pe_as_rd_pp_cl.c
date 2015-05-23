/*
 * Copyright (c) 1994-2015 Nineties Retro
 *
 * A simple AS pretty-printer.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include "pe_as_rd.h"
#include "pe_as_rd_pp.h"


static char const *program_name;

/*
 * If the <indentation> is not defined, then a default value is assumed :-
 */
static int indentation= 2;

static char const *processed_file_prefix;
static char const *language_prefix = "peg.";


static void
usage(void)
{
	fprintf(stderr, "%s: [ -i<indentation> ] [ -l<language> ] processed-file-prefix\n", program_name);
}

static int
process_arguments(int argc, char **argv)
{
	program_name = argv[0];
	while (++argv, --argc != 0 && **argv == '-') {
		switch((*argv)[1]) {
		case 'i':
			indentation = atoi(&(*argv)[2]);
			if (indentation < 0) {
				fprintf(stderr, "%s: %s not a valid indentation level", program_name, &(*argv)[2]);
				return EINVAL;
			}
			break;
		case 'l':
			language_prefix= &(*argv)[2];
			break;
		case 'h':
		default:
			usage();
			return EINVAL;
		}
	}
	if (argc != 1) {
		usage();
		return EINVAL;
	}
	processed_file_prefix = argv[0];
	return 0;
}


int main(int argc, char ** argv)
{
	struct pe_as_rd reader;
	struct pe_as_rd_pp pp;
	int err;

	err = process_arguments(argc, argv);
	if (err)
		goto could_not_process_arguments;
	pe_as_rd_open(&reader);
	pe_as_rd_language(&reader, language_prefix);
	err = pe_as_rd_data(&reader, processed_file_prefix);
	if (err)
		goto could_not_read_data;
	pe_as_rd_pp_open(&pp, &reader);
	pe_as_rd_pp_indentation(&pp, indentation);
	err = pe_as_rd_pp_display(&pp, pe_as_rd_root(&reader));
	if (err)
		goto could_not_display;
	pe_as_rd_pp_close(&pp);
	pe_as_rd_close(&reader);
	return EXIT_SUCCESS;
	
could_not_display:
	pe_as_rd_pp_close(&pp);
	pe_as_rd_close(&reader);
could_not_read_data:
could_not_process_arguments:
	return err;
}
