#define _POSIX_SOURCE 1

/*******************************************************************************
/***	get_char.c	Module for LASH		Gary Johnson	1993
/***	Module of LASH deals with handling of user input. 
/***	Converts the character into a token and returns with character typed.
/******************************************************************************/

#include<termios.h>
#include<sys/types.h>
#include<unistd.h>
#include<stdio.h>
#include"../Panic/panic.h"

/*******************************************************************************
/***  Macro definitions for character type checking

	/* Normal Character */
#define ISCHAR(c) 	(isalnum(c) || ispunct(c))
	/* New Line Character */
#define ISRETN(c)	(c=='\n')
	/* Tab Character */
#define ISTABB(c)	(c=='\t')
	/* Back Space Character */
#define ISEDIT(c)	(c=='\b')
	/* Space Character */
#define ISSPAC(c)	(c==' ')
	/* CTRL-D Quit Character */
#define ISQUIT(c)	(c=='\4')
	/* CTRL-P/N History Characters */
#define ISHIST(c)	(c=='\20'||c=='\16')
/******************************************************************************/

/* Tokens for Return to caller: defined as per Macro definitions */
typedef enum
  {
    NORM, RETN, TABB, EDIT, SPAC, HIST, QUIT, FAIL
  }
TOKEN;

/* File Scope variables for tty mode amendments */
static short raw_ready;		/* If in raw mode set to 1 else set to 0 */
static struct termios t_old;	/* Holds copy of sane tty mode as per start */

/*******************************************************************************
/***	void set_to_raw ()
/***	Called in preparation of command being typed to stop echo and to force
/***	characters to be returned one-by-one as typed.
/***	Stores previous mode in t_old
/******************************************************************************/
void 
set_to_raw ()
{
  struct termios t;
  if (tcgetattr (STDIN_FILENO, &t) != 0)
    PANIC;
  t_old = t;
  t.c_cc[VMIN] = 1;
  t.c_cc[VTIME] = 0;
  t.c_lflag &= ~(ICANON		/* Returns characters individually */
		 | ECHO | ECHOE | ECHOK | ECHONL);	/* Stops char echo */
  if (tcsetattr (STDIN_FILENO, TCSANOW, &t) == -1)
    PANIC;
  raw_ready = 1;
};

/*******************************************************************************
/***	void set_to_run ()
/***	Called after a command has been entered before being executed to reset
/***	input to normal before calling command
/******************************************************************************/
void 
set_to_run ()
{
  if (tcsetattr (STDIN_FILENO, TCSADRAIN, &t_old) == -1)
    PANIC;
  raw_ready = 0;
};

/*******************************************************************************
/***	TOKEN get_char (char *c)
/***	Waits for and returns one character from the user - first translating
/***	into a token for return.
/******************************************************************************/
TOKEN 
get_char (char *c)
{
  TOKEN ret_token;
  if (!(raw_ready))
    set_to_raw ();

  read (STDIN_FILENO, c, 1);

  if (ISCHAR (*c))
    ret_token = NORM;
  else if (ISEDIT (*c))
    ret_token = EDIT;
  else if (ISRETN (*c))
    ret_token = RETN;
  else if (ISTABB (*c))
    ret_token = TABB;
  else if (ISHIST (*c))
    ret_token = HIST;
  else if (ISSPAC (*c))
    ret_token = SPAC;
  else if (ISQUIT (*c))
    ret_token = QUIT;
  /* No Match */
  else
    ret_token = FAIL;

  return (ret_token);
};
