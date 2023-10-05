/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set et sw=2 ts=2: */
/***************************************************************************
 *            debug_filter.c
 *
 *  Fri Dec  7 14:24:27 CET 2012
 *  Copyright 2012 Bent Bisballe Nyeng
 *  deva@aasimon.org
 ****************************************************************************/

/*
 *  This file is part of Hugin.
 *
 *  Hugin is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Hugin is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Hugin; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
 */
#include "hugin_filter.h"

#ifndef DISABLE_HUGIN

#include <string.h>
#include <stdlib.h>

#ifndef WITH_HUG_FILTER
#warning hugin_filter.c compiled but WITH_HUG_FILTER not defined
#endif

#define NELEM(x)	(sizeof(x)/sizeof((x)[0]))
struct __debug_channel
{
	char name[32];
	unsigned flags;
};

#define __DEBUG_CHANNEL_MAX 256

static struct __debug_channel debug_channel[__DEBUG_CHANNEL_MAX];
static unsigned n_debug_channel = 0;

// Default is to enable everything...
static unsigned debug_flags = 0xffffffff;//(1<<__class_err)|(1<<__class_fixme);

int hug_filter_enabled(const enum __debug_class cl, const char *ch)
{
	unsigned i;
	for(i = 0; i < n_debug_channel; i++) {
		if(!strcmp(ch, debug_channel[i].name)) {
			return (debug_channel[i].flags & (1 << cl)) != 0;
		}
	}
	return debug_flags & (1 << cl);
}

/*
 * fmt := [set[,set]*]*
 * set := [+-]channel
 *     |  class[+-]channel
 *     |  [+-]all
 */
int hug_filter_parse(const char *filter)
{
	char *s;
	char *next;
	char *opt;

  // NOTE: This must be identical to the debug_class_str in debug.c
  const char * const debug_class_str[] =
    { "fixme", "err", "warn", "info", "debug" };

	if(!(s = strdup(filter))) return 1;

	for(opt = s; opt; opt = next) {
		int set = 0;
		int clr = 0;
		unsigned i;
		if((next = strchr(opt, ','))) *next++ = '\0';
		char *p = opt + strcspn(opt, "+-");
		if(!*p) p = opt;	// All chars -> a channel name
		if(p > opt) {
			// we have a class
			for(i = 0; i < NELEM(debug_class_str); i++) {
				int n = strlen(debug_class_str[i]);
				if(n != (p - opt)) continue;
				if(!memcmp(opt, debug_class_str[i], n)) {
					// Found the class
					if(*p == '+')
						set = 1 << i;
					else
						clr = 1 << i;
					break;
				}
			}
			if(i == NELEM(debug_class_str)) continue;
		} else {
			if(*p == '-')
				clr = ~0;
			else
				set = ~0;
		}
		if(*p == '+' || *p == '-') p++;
		if(!*p) continue;
		if(!strcmp("all", p)) {
			debug_flags = (debug_flags & ~clr) | set;
		} else {
			if(strlen(p) >= sizeof(debug_channel[0].name)) continue;
			for(i = 0; i < n_debug_channel; i++) {
				if(!strcmp(p, debug_channel[i].name)) {
					debug_channel[i].flags = (debug_channel[i].flags & ~clr) | set;
					break;
				}
			}
			if(i == n_debug_channel && n_debug_channel < __DEBUG_CHANNEL_MAX) {
				strcpy(debug_channel[i].name, p);
				debug_channel[i].flags = (debug_flags & ~clr) | set;
				n_debug_channel++;
			}
		}
	}
	free(s);

  return 0;
}

#endif/*DISABLE_HUGIN*/
