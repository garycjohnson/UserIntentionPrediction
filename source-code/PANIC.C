#define _POSIX_SOURCE 1

/*******************************************************************************
/*** panic.c	Module for LASH		Gary Johnson	1994
/*** Support function for LASH called when an unrecoverable error is found
/*** Prints the name and line number in the source file at which the error
/*** occurred.  Finishes by aborting program execution.
/******************************************************************************/

#include <stdlib.h>
#include <stdio.h>

void 
panic (char *filename, int line)
{
  (void) fprintf (stderr, "\n Panic in line %d of file %s\n", line, filename);
  (void) perror ("Unexpected library error");
  abort ();
}
