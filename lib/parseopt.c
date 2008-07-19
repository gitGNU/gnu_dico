/* This file is part of Dico.
   Copyright (C) 2008 Sergey Poznyakoff

   Dico is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   Dico is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Dico.  If not, see <http://www.gnu.org/licenses/>. */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <dico.h>
#include <string.h>
#include <libi18n.h>

static struct dico_option *
find_opt(struct dico_option *opt, const char *str, const char **value)
{
    size_t len = strlen(str);
    int isbool;
	
    if (len > 2 && memcmp(str, "no", 2) == 0) {
	*value = NULL;
	str += 2;
	isbool = 1;
    } else {
	isbool = 0;
	*value = str;
    }
	
    for (; opt->name; opt++) {
	if (len >= opt->len
	    && memcmp(opt->name, str, opt->len) == 0
	    && (!isbool || opt->type == dico_opt_bool)) {
	    int eq = str[opt->len] == '=';
	    
	    switch (opt->type) {
	    case dico_opt_long:
	    case dico_opt_string:
	    case dico_opt_const_string:
	    case dico_opt_enum:
		if (!eq)
		    continue;
		*value = str + opt->len + 1;
		break;

	    case dico_opt_null:
		if (eq)
		    *value = str + opt->len + 1;
		else
		    *value = NULL;
		break;					
				
	    default:
		if (eq)
		    continue;
		break;
	    }
	    return opt;
	}
    }
    return NULL;
}

int
find_value(const char **enumstr, const char *value)
{
    int i;
    for (i = 0; *enumstr; enumstr++, i++)
	if (strcmp(*enumstr, value) == 0)
	    return i;
    return -1;
}

int
dico_parseopt(struct dico_option *opt, int argc, char **argv)
{
    long n;
    char *s;
    int rc = 0;
    const char *modname = argv[0];

    _dico_libi18n_init();
    for (argc--, argv++; argc-- > 0; ++argv) {
	const char *value;
	struct dico_option *p = find_opt(opt, *argv, &value);
	
	if (!p) {
	    dico_log(L_ERR, 0, _("%s: %s: unknown option"), modname, *argv);
	    rc = 1;
	    continue;
	}
				
	switch (p->type) {
	case dico_opt_long:
	    n = strtol(value, &s, 0);
	    if (*s) {
		dico_log(L_ERR, 0, _("%s: %s: %s is not a valid number"),
			 modname, p->name, value);
		rc = 1;
		continue;
	    }
	    *(long*)p->data = n;
	    break;
	    
	case dico_opt_const:
	    *(long*)p->data = p->v.value;
	    break;
	    
	case dico_opt_const_string:
	    *(const char**)p->data = value;
	    break;
	    
	case dico_opt_string:
	    *(const char**)p->data = strdup(value);
	    break;
	    
	case dico_opt_bool:
	    if (p->v.value) {
		if (value)
		    *(int*)p->data |= p->v.value;
		else
		    *(int*)p->data &= ~p->v.value;
	    } else
		*(int*)p->data = value != NULL;
	    break;
	    
	case dico_opt_bitmask:
	    *(int*)p->data |= p->v.value;
	    break;
				
	case dico_opt_bitmask_rev:
	    *(int*)p->data &= ~p->v.value;
	    break;
	    
	case dico_opt_enum:
	    n = find_value(p->v.enumstr, value);
	    if (n == -1) {
		dico_log(L_ERR, 0, _("%s: %s: invalid value %s"),
			 modname, p->name, value);
		rc = 1;
		continue;
	    }
	    *(int*)p->data = n;
	    break;

	case dico_opt_null:
	    break;
	}

	if (p->func && p->func (p, value))
	    rc = 1;
    }
    return rc;
}
