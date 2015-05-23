#ifndef pe_parser_h
#define pe_parser_h

/*
 * Copyright (c) 1995-2015 Nineties-Retro
 */

/* XH: stdio.h */
/* XH: pe_grammar.h */
/* XH: pe_lexer.h */

struct pe_parser {
	struct pe_lexeme lexeme;
	struct pe_lexer *lexer;
	struct pe_grammar *grammar;
	FILE           *error_sink;
	int (*parse_error)(unsigned long);
};

void
pe_parser_open(struct pe_parser *, struct pe_lexer *, struct pe_grammar *);

void
pe_parser_error_sink(struct pe_parser *, int (*)(unsigned long), FILE *);

int
pe_parser_parse(struct pe_parser *);

void
pe_parser_close(struct pe_parser *);

#endif
