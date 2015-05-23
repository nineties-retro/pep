/*
 * Copyright (c) 1995-2015 Nineties-Retro
 */

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include "pe_as_kw.h"
#include "pe_rule_set.h"

void
pe_rule_set_open(struct pe_rule_set *rs)
{
	rs->head = 0;
}


static int
pe_rule_set_member(struct pe_rule_set *rs, pe_as_kw kw, unsigned long *line)
{
	struct pe_rule_set_node *n;

	for (n = rs->head; n != 0; n = n->next) {
		if (pe_as_kw_eq(n->kw, kw)) {
			*line = n->line;
			return 1;
		}
	}
	return 0;
}



int
pe_rule_set_insert(struct pe_rule_set *rs, pe_as_kw kw, unsigned long *line)
{
	if (pe_rule_set_member(rs, kw, line)) {
		return 0;
	} else {
		struct pe_rule_set_node *n = malloc(sizeof(*n));

		if (n == 0)
			return ENOMEM;
		n->next = rs->head;
		n->kw = kw;
		n->line = *line;
		rs->head = n;
		return -1;
	}
}


void
pe_rule_set_close(struct pe_rule_set *rs)
{
	struct pe_rule_set_node *n, *p;

	for (p = rs->head; p != 0; p= n) {
		n = p->next;
		free(p);
	}
}
