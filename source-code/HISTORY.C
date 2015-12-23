#define _POSIX_SOURCE 1

#include<string.h>
#include<malloc.h>
#include<stdio.h>
#include"../Panic/panic.h"

/* Number of commands held in history */
#define HISTSIZE 5
/* Maximum number of arguments in a command line */
#define LINEARGS 20


/* Derived from main.c - current command line */
extern char *arg[LINEARGS];
/* Current capacity of history table (Max=HISTSIZE) */
       short ht_capacity;
/* Array of command lines - the actual history store */
static char *hist_table[HISTSIZE][LINEARGS];

/*******************************************************************************
/***	void retr_hist (short, short)
/***	This function replaces the current command line with the history 
/***	line as selected by the posn(0->HISTSIZE-1).  idx is also updated
/***	and this is the number of arguments contained in the new command line
/******************************************************************************/
void
retr_hist (short posn, short *idx)
{
  short count = 0;
  short histloc;

  /* If -1 is sent as posn make all arguments empty and return empty command */
  if (posn == -1)
    {
      strcpy (arg[0], "");
      for (count = 1; count <= *idx; count++)
	{
	  free (arg[count]);
	  arg[count] = NULL;
	}
      *idx = 0;
    }
  /* otherwise return the full command line from the history list */
  else if (posn < ht_capacity)
    {
      /* history is stored numerically backwards so correct that */
      histloc = ht_capacity - posn - 1;
      while (hist_table[histloc][count] != NULL)
        /* for each argument in the command */
	{
          /* (re)allocate memory dependent on current status */
	  if (arg[count] == NULL)
	    arg[count] = malloc
	      ((strlen (hist_table[histloc][count]) + 1) * sizeof (char));
	  else
	    realloc (arg[count], (1 + strlen (
			      hist_table[histloc][count])) * sizeof (char));
	  /* and copy the new argument from history to current */
	  strcpy (arg[count], hist_table[histloc][count]);
	  count++;
	}
      /* return the number of arguments */
      *idx = count - 1;
    }
}


/*******************************************************************************
/***	void list_hist ()
/***	This function is activated by the !@lh shell command.  It lists the 
/***	current contents of the history list to the user.
/******************************************************************************/
void
list_hist ()
{
  short count, count2;
  for (count = 0; count < ht_capacity; count++)
  /* for every line in the history table */
    {
      count2 = 0;
      while (hist_table[count][count2] != NULL)
      /* and for every argument in that line */
	{
          /* print the argument to the screen */
	  printf ("%s ", hist_table[count][count2]);
	  count2++;
	}
      /* separating each line with a newline character */
      printf ("\n");
    }
}

/*******************************************************************************
/***	void add_hist ()
/***	This function adds a new command line to the history.  In normal use 
/***	this function would be called after each command is executed.
/******************************************************************************/
void
add_hist ()
{
  short count, count2, len;

  if (ht_capacity < HISTSIZE)
  /* CASE 1: history store is not full */
    {
      count = 0;
      while (arg[count] != NULL)
      /* for each argument in command executed */
	{
          /* allocate memory for the data store */
	  len = strlen (arg[count]);
	  if ((hist_table[ht_capacity][count] =
	       malloc ((len + 1) * sizeof (char))) == NULL)
	      PANIC;
          /* put the contents of the command into the table */
	  strcpy (hist_table[ht_capacity][count], arg[count]);
	  count++;
	}
      /* add the end signifier to the line */
      hist_table[ht_capacity][count] = NULL;
      /* history is now one fuller */
      ht_capacity++;
    }
  else
  /* CASE 2: history store is at HISTSIZE */
    {
      count = 0;
      /* free all the memory used by the 'to be removed' entry */
      while (hist_table[0][count] != NULL)
	{
	  free (hist_table[0][count]);
	  count++;
	}
      /* re-point all the other entries in the history to one line older */
      for (count = 1; count < ht_capacity; count++)
      /* for each line in table */
	{
	  count2 = 0;
	  while (hist_table[count][count2] != NULL)
          /* for each argument in line */
	    {
              /* re-point to one older */
	      hist_table[count - 1][count2] =
		hist_table[count][count2];
	      count2++;
	    }
	  hist_table[count - 1][count2] = NULL;
	}
      count = 0;
      /* now add the new command line to the empty space created */
      while (arg[count] != NULL)
      /* for each argument in the line */
	{
          /* create space for the argument */
	  len = strlen (arg[count]);
	  if ((hist_table[ht_capacity - 1][count] =
	       malloc ((len + 1) * sizeof (char))) == NULL)
	      PANIC;
          /* put the argument into the table */
	  strcpy (hist_table[ht_capacity - 1][count], arg[count]);
	  count++;
	}
      /* and finish it off with a NULL to signify the end */
      hist_table[ht_capacity - 1][count] = NULL;
    }
}
