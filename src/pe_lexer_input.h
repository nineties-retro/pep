#ifndef pe_lexer_input_h
#define pe_lexer_input_h

/*
 * Copyright (c) 1994-2015 Nineties Retro
 */

#include <stdio.h>

typedef FILE *pe_lexer_input;

#define pe_lexer_input_read(in, n, buff) \
	fread(buff, 1, n, in)

#define pe_lexer_input_open(in, file_name) \
	((in= fopen(file_name, "r")) != (FILE *)0)

#define pe_lexer_input_close(in) \
	(fclose(in) == 0)

#endif
