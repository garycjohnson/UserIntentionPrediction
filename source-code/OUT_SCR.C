#define _POSIX_SOURCE 1

/*******************************************************************************
/***	out_scr.c	Module for LASH		Gary Johnson	1993
/***	Called from main routine to display command typed so far (with any
/***	predictions).  Also outputs prompt message.
/******************************************************************************/

#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<time.h>
#include<malloc.h>
#include<stdlib.h>
#include"../Panic/panic.h"

/* Prompt symbol used after host and time */
#define PROMPT "> "

/* Holds a copy of the command string last time it was written to screen */
static char *old_str;
/* Holds the name of the host machine for display in prompt */
static char *mc_name;
/* call_once ==1 iff the out_prompt function has been called */
static short call_once;

/*******************************************************************************
/***	void out_prompt ()
/***	Displays command line prompt - asking user for input
/***	Prompt has format:	$HOST $TIME PROMPT
/******************************************************************************/
void
out_prompt ()
{
  time_t num_time;
  struct tm *str_time;
  char *chr_time;

  if (call_once != 1)
    {
      /* Set mc_name to be the name of the host machine */
      if ((mc_name = malloc ((strlen (getenv ("HOST")) + 1) * sizeof (char))) == NULL)
	  PANIC;
      strcpy (mc_name, (getenv ("HOST")));
      call_once = 1;
    }
  else
    free (old_str);
  old_str = NULL;

  /* Calculate current time and format into output-able form */
  if ((chr_time = malloc ((9 + 1) * sizeof (char))) == NULL)
      PANIC;
  num_time = time ((time_t *) NULL);
  str_time = localtime (&num_time);
  strftime (chr_time, sizeof (char) * 11, " %a %H:%M", str_time);

  /* Write the prompt to the screen: $HOST $TIME PROMPT */
  write (STDOUT_FILENO, mc_name, strlen (mc_name));
  write (STDOUT_FILENO, chr_time, strlen (chr_time));
  write (STDOUT_FILENO, PROMPT, strlen (PROMPT));
}

/*******************************************************************************
/***	void out_line (char **line_args, short i)
/***	Displays the command as typed so far to the user
/***	Overview: Calculates difference between last output and current, 
/***	create a amending string and output to correct the output
/******************************************************************************/
void
out_line (char *new_str)
{
  short count1 = 0, count2, old_len = 0, new_len, out_str_len = 0, len_so_far;
  char *out_str;

  new_len = strlen (new_str);
  if (old_str != NULL)
    {
      old_len = strlen (old_str);
      while ((old_str[count1] == new_str[count1]) && old_str[count1] != '\0')
	count1++;
    }

  out_str_len += old_len - count1;
  out_str_len += new_len - count1;
  if (old_len > new_len)
    out_str_len += 2 * (old_len - new_len);
  if ((out_str = malloc ((out_str_len + 1) * sizeof (char))) == NULL)
      PANIC;

  for (count2 = 0; count2 < (old_len - count1); count2++)
    out_str[count2] = '\b';
  len_so_far = old_len - count1;
  for (count2 = 0; count2 < (new_len - count1); count2++)
    out_str[count2 + len_so_far] = new_str[count1 + count2];
  len_so_far += new_len - count1;
  if (old_len > new_len)
    {
      for (count2 = 0; count2 < (old_len - new_len); count2++)
	out_str[count2 + len_so_far] = ' ';
      len_so_far += old_len - new_len;
      for (count2 = 0; count2 < (old_len - new_len); count2++)
	out_str[count2 + len_so_far] = '\b';
      len_so_far += old_len - new_len;
    }
  out_str[len_so_far] = '\0';
  write (STDOUT_FILENO, out_str, len_so_far);

  free (out_str);
  if (old_str != NULL)
    free (old_str);
  if ((old_str = malloc ((new_len + 1) * sizeof (char))) == NULL)
    PANIC;
  strcpy (old_str, new_str);
}
