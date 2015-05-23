#ifndef pe_rule_set_h
#define pe_rule_set_h

/*
 * Copyright (c) 1995-2015 Nineties-Retro
 *
 * Simple link-list based set.
 */

/* XH: pe_as_kw.h */

struct pe_rule_set_node {
	struct pe_rule_set_node * next;
	pe_as_kw                  kw;
	unsigned long             line;
};

struct pe_rule_set {
	struct pe_rule_set_node *head;
};

void
pe_rule_set_open(struct pe_rule_set *);

int
pe_rule_set_insert(struct pe_rule_set *, pe_as_kw, unsigned long *);

void
pe_rule_set_close(struct pe_rule_set *);

#endif