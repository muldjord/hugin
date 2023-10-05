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

#define MULTMSG  \
"Lorem ipsum dolor sit amet, consectetur adipiscing elit. Aenean sed mauris id mauris commodo faucibus eu at dui. Duis lorem est, auctor sed tellus nec, aliquam blandit sapien. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Mauris mattis dolor elit, in vulputate est mattis sed. Morbi vel ligula ut dui hendrerit congue id at dolor. Suspendisse vitae arcu aliquam, laoreet lectus a, faucibus libero. Nunc consectetur turpis a suscipit vulputate. Fusce quis libero id mi scelerisque ultricies. Nullam consectetur lectus non augue mollis consequat. Donec in ipsum imperdiet, blandit eros vitae, pretium erat. Suspendisse dolor neque, sodales a condimentum vitae, placerat vel lorem. Aliquam bibendum tempus metus, ac tincidunt ipsum placerat in. Phasellus blandit, ligula nec imperdiet eleifend, justo tortor volutpat lectus, sed lacinia lectus orci suscipit erat. Aliquam sollicitudin consequat justo, sit amet malesuada diam luctus quis. Donec imperdiet tortor nec egestas euismod. Etiam rhoncus pellentesque dolor, quis luctus felis mollis sit amet." \
"Nullam auctor erat metus, eget venenatis leo sagittis ac. Suspendisse sit amet velit sodales, gravida felis nec, ornare felis. Aliquam erat volutpat. Morbi a vehicula nunc, non aliquam augue. Nunc quis ligula et nunc malesuada fringilla non at tortor. Nam pretium molestie sapien, eget ultrices nunc imperdiet vitae. Sed sed odio id diam scelerisque vestibulum in a neque. Maecenas aliquam placerat ornare. Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos himenaeos. Interdum et malesuada fames ac ante ipsum primis in faucibus. Proin accumsan sodales augue. Pellentesque sagittis tincidunt lorem, ac sagittis lorem posuere non. In tincidunt nisi lacinia nisi aliquam auctor et ut felis. In eget consequat leo. Donec at faucibus ligula." \
"Aliquam non libero rutrum, euismod magna vitae, auctor tortor. Quisque accumsan fringilla neque. Nulla facilisi. Curabitur ante risus, interdum vitae mi in, suscipit commodo mi. Etiam in mi volutpat orci tincidunt posuere."

int main(int argc, char *argv[])
{
  hug_status_t status = hug_init(HUG_FLAG_OUTPUT_TO_SYSLOG, 
                                 HUG_OPTION_SYSLOG_HOST, "localhost",
                                 HUG_OPTION_SYSLOG_PORT, 514,
                                 HUG_OPTION_END);

  if(status != HUG_STATUS_OK) {
    printf("Error: %d\n", status);
    return 1;
  }

  INFO(example, "We are up and running");

  DEBUG(example, "Or are we %d?", 42);

  DEBUG(foo, "Or are we %d?", 42);

  DEBUG(crop, "%s", MULTMSG);

  hug_close();

  status = hug_init(HUG_FLAG_OUTPUT_TO_SYSLOG, 
                    HUG_OPTION_SYSLOG_HOST, "notahost",
                    HUG_OPTION_SYSLOG_PORT, 514,
                    HUG_OPTION_END);

  if(status != HUG_STATUS_OK) {
    printf("Error: %d\n", status);
    return 1;
  }

  return 0;
}
