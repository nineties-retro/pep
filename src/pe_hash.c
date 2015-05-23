/*
 * Copyright (c) 1994-2015 Nineties Retro
 */

#include <stddef.h>
#include "pe_hash.h"

pe_hash
pe_hash_lit(size_t len, char const *str)
{
	pe_hash_vars(0);
	while (len != 0) {
		len -= 1;
		pe_hash_char(*str);
		str += 1;
	}
	return pe_hash_value;
}
