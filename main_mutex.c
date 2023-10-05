/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set et sw=2 ts=2: */
/***************************************************************************
 *            main.cc
 *
 *  Fri Dec  7 09:35:45 CET 2012
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
#include "hugin.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdio.h>

int main(int argc, char *argv[])
{
  hug_status_t status = hug_init(HUG_FLAG_USE_MUTEX |
                                 HUG_FLAG_OUTPUT_TO_STDOUT |
                                 0,
                                 HUG_OPTION_END);

  if(status != HUG_STATUS_OK) {
    printf("Error: %d\n", status);
    return 1;
  }

  INFO(example, "We are up and running");

  DEBUG(example, "Or are we %d?", 42);

  DEBUG(foo, "Or are we %d?", 42);
  
  hug_close();

  return 0;
}
