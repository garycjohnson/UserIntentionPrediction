#define _POSIX_SOURCE 1

/*******************************************************************************
/***	exec_com.c	Module of LASH     Gary Johnson     1994
/***	exec_com.c is called with a Unix command line.  
/***	This is then executed and a value returned depicting the success(0)
/***	or failure(not 0) of the command.
/***	The built-in command cd is handled separately.
/******************************************************************************/

#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>
#include<limits.h>
#include<stdlib.h>
#include<signal.h>
#include<string.h>
#include<errno.h>

/***	Contains program abortion routine triggered by unrecoverable errors ***/
#include"../Panic/panic.h"


/*******************************************************************************
/***	int call_chdir (char **buf)
/***	If run_command is called with the command of 'cd' this function is
/***	called.
/***    The second argument is then taken to be the new path (relative or
/***	absolute) and the working directory is changed to the new one.
/******************************************************************************/
int 
call_chdir (char **buf)
{

  errno = 0;
  if (buf[1] == NULL)
  /* First case: no directory argument; go home */
    chdir (getenv ("HOME"));
  else
  /* Second general case: directory argument given; change to this dir */
    chdir (buf[1]);

  return (errno);
}

/******************************************************************************
/***	int run_command (char **buf)
/***	Called from the main routine to execute the typed command
/***	If command is 'cd' calls call_chdir else executes command
/***	Creates new process with fork and then executes command whilst
/***	parent waits for process to end
/*****************************************************************************/
int 
run_command (char **buf)
{
  int status, pid;

  if (!(strcmp (buf[0], "\0")) == 0)

    if (strcmp (buf[0], "cd") == 0)
    /* Catch calls to 'cd' and perform directly */
      return (call_chdir (buf));
    else
      {
	pid = fork ();
	/* Create new process and then check for success */
	switch (pid)
	  {
	  case -1:
	  /* Case -1: Failed to create new process */
	    PANIC;
	  case 0:
          /* Case 0: Child enters here on success and executes command */
	    execvp (buf[0], buf);
	    perror (buf[0]);
	    exit (1);
	  default:
	  /* Default: Parent enters here and waits for child to exit */
	    wait (&status);
	    return (WEXITSTATUS (status));
	  }
      }
  return (1);
};
