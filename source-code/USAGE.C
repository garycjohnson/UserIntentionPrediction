#define _POSIX_SOURCE 1

/*******************************************************************************
/*** usage.c	Module for LASH		Gary Johnson	1994
/*** Support functions to print help and usage messages for LASH
/******************************************************************************/

#include <stdio.h>

/*******************************************************************************
/*** void shell_com_help ()
/*** Prints help message for !@ Shell Commands
/******************************************************************************/
void 
shell_com_help ()
{

  printf ("\n");
  printf ("\t!@ Shell Commands Help.\n");
  printf ("These commands are used to alter the probabilities of the commands\n");
  printf ("and arguments used by LASH.\n\n");

  printf ("\t!@he Show this message.\n");

  printf ("\t!@lc List all commands in table.\n");
  printf ("\t!@ld List all directories in table.\n");
  printf ("\t!@lh List the history table.\n");
  printf ("\t!@la <command> List arguments for command.\n");

  printf ("\t!@rc <command> Remove the <command> from command list.\n");
  printf ("\t!@rd <directory> Remove the <directory> from directory list.\n");
  printf ("\t!@ra <command> <argument> Remove argument from asspciated command.\n");
  printf ("\t!@[+/-]c <command> <value> Change the frequency of command.\n");
  printf ("\t!@[+/-]a <command> <argument> <value> As above for arguments.\n");

  printf ("\n");

}

/*******************************************************************************
/*** void print_usage (char *comname)
/*** Displays usage message to user if lash is called with incorrect arguments
/******************************************************************************/
void 
print_usage (char *comname)
/* comname: string equal to the name used to call lash */
{

  printf ("Usage: %s [-f <filename>]\n", comname);

}
