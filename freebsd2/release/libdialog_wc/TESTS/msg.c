/*
 * small test-driver for new dialog functionality
 *
 * Copyright (c) 1995, Jordan Hubbard
 *
 * All rights reserved.
 *
 * This source code may be used, modified, copied, distributed, and
 * sold, in both source and binary form provided that the above
 * copyright and these terms are retained, verbatim, as the first
 * lines of this file.  Under no circumstances is the author
 * responsible for the proper functioning of the software nor does
 * the author assume any responsibility for damages incurred with
 * its use.
 *
 * $Id: msg.c,v 1.1 1999/09/13 08:50:21 itojun Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <dialog.h>

/* Kick it off, James! */
int
main(int argc, unsigned char *argv[])
{
  int retval;

  init_dialog();

  retval = dialog_msgbox("This is dialog_msgbox() in action with pause on", "Hi there.  Please press return now.",
			 -1, -1, 1);
  dialog_clear();
  fprintf(stderr, "returned value for dialog_msgbox was %d\n", retval);

  end_dialog();
  return 0;
}
