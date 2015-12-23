#define _POSIX_SOURCE 1

#include<signal.h>
#include<stdio.h>
#include<string.h>
#include<malloc.h>
#include<unistd.h>

#include"../Panic/panic.h"
#include"../Predict/predictor.h"
#include"../Data/data_store.h"
#include"../Data/constants.h"
#include"../Messages/usage.h"
#include"../History/history.h"
#include"debug_defs.h"

#ifdef DEBUG_FILE
#define DB_FILENAME "/dcs/91/gjohnson/Project/DebugFile"
#endif

#define DEFAULT_FILENAME ".lashdata"
#define HISTSIZE 5
#define LINEARGS 20
typedef enum
  {
    NORM, RETN, TABB, EDIT, SPAC, HIST, QUIT, FAIL
  }
TOKEN;


extern void out_line (char *);
extern void out_prompt ();
extern TOKEN get_char (char);
extern void set_to_run ();
extern int run_command (char **);

short dir_ref;
short com_ref;
char *arg[LINEARGS];
#ifdef DEBUG_FILE
FILE *dbf;
#endif

/*** Check for user arguments and set filename as required ***/
short user_args(int argc, char **argv) {
  char *temp_str;
  short i;
  if((temp_str=malloc((256+1)*sizeof(char)))==NULL) PANIC;

  if (argc == 1)
  /* No flags given */
    {
      strcpy (temp_str, (char *) getenv ("HOME"));
      i = strlen (temp_str) + strlen (DEFAULT_FILENAME) + 1;
      if ((filename = malloc ((i + 1) * sizeof (char))) == NULL)
	  PANIC;
      strcpy (filename, temp_str);
      strcat (filename, "/");
      strcat (filename, DEFAULT_FILENAME);
    }
  else if ((argc == 3) && (strcmp (argv[1], "-f") == 0))
  /* Correct flags given */
    if (argv[2][0] == '/')
    /* Full pathname */
      {
	if ((filename = malloc ((1 + strlen (argv[2])) * sizeof (char)))
	    == NULL)
	    PANIC;
	strcpy (filename, argv[2]);
      }
    else
    /* Relative pathname */
      {
	getcwd (temp_str, 256);
	strcat (temp_str, "/");
	strcat (temp_str, argv[2]);
	if ((filename = malloc ((1 + strlen (temp_str)) * sizeof (char)))
	    == NULL)
	    PANIC;
	strcpy (filename, temp_str);
      }
  else
  /* Illegal command line form */
    {
      print_usage (argv[0]);
      return (1);
    }
  return(0);
}

/* EDIT case: remove last character */
void
case_edit (short *x)
{
  int len;
  len = strlen (arg[*x]);
  if (len > 0)
    *(arg[*x] + len - 1) = '\0';
  else if (*x > 0)
    *x = *x - 1;

}

/* Calling function for !@ shell commands */
short
shell_commands (char **sh_com, short count)
{
  short illegal = 0;

  if (count == 0)
/*** Commands taking 0 arguments ***/
    if (strcmp (sh_com[0], "!@he") == 0)
      shell_com_help ();
    else if (strcmp (sh_com[0], "!@lc") == 0)
      list_coms ();
    else if (strcmp (sh_com[0], "!@ld") == 0)
      list_dirs ();
    else if (strcmp (sh_com[0], "!@lh") == 0)
      list_hist ();
    else
      illegal = 1;
  else if (count == 1)
/*** Commands taking 1 argument ***/
    if (strcmp (sh_com[0], "!@rc") == 0)
      illegal = rem_command (sh_com[1]);
    else if (strcmp (sh_com[0], "!@rd") == 0)
      illegal = rem_dir (sh_com[1]);
    else if (strcmp (sh_com[0], "!@la") == 0)
      illegal = list_args (sh_com[1]);
    else
      illegal = 1;
  else if (count == 2)
/*** Commands taking 2 arguments ***/
    if (strcmp (sh_com[0], "!@+c") == 0)
      illegal = change_prob_com (sh_com[1], sh_com[2], 1);
    else if (strcmp (sh_com[0], "!@-c") == 0)
      illegal = change_prob_com (sh_com[1], sh_com[2], -1);
    else if (strcmp (sh_com[0], "!@ra") == 0)
      illegal = rem_argument (sh_com[1], sh_com[2]);
    else
      illegal = 1;
  else if (count == 3)
/*** Commands taking 3 arguments ***/
    if (strcmp (sh_com[0], "!@+a") == 0)
      illegal = change_prob_arg (sh_com[1], sh_com[2], sh_com[3], 1);
    else if (strcmp (sh_com[0], "!@-a") == 0)
      illegal = change_prob_arg (sh_com[1], sh_com[2], sh_com[3], -1);
    else
      illegal = 1;
  else
    illegal = 1;

  if (illegal != 0)
  /* An error has occured */
    {
      return (-1);
    }

  return (0);
}

/* Calling function for predict_com function */
short
call_pred_com ()
{
  if ((com_ref = predict_com (arg[0])) == -1)
  /* No prediction made */
    return (0);
  else
  /* Prediction made: substitute for current arg */
    {
      strcpy (arg[0], (char *) get_com_name (com_ref));
      return (1);
    }
}

/* Calling function for predict_arg function */
short
call_pred_arg (short idx)
{
  short ret;
  ret = predict_arg (arg[idx], com_ref);
  if (ret == -1)
  /* No prediction made */
    return (0);
  else
  /* Prediction made: substiture for arg */
    {
      if (ret < NUM_ARGS)
	strcpy (arg[idx], (char *) get_arg_name (com_ref, ret));
      else
	strcpy (arg[idx], (char *) get_arg_tt (ret - NUM_ARGS));
      return (1);
    }
}

/* MAIN PROCEDURE */
int
main (int argc, char *argv[])
{
  char *st, ch, *old_st, *dir_name;
  short i, j, count, guess_active = 0, histcount = -1;
  char *line = NULL;
  TOKEN rectok;

  struct sigaction act, oact;
  act.sa_handler = SIG_IGN;

  if ((st = malloc ((1 + 1) * sizeof (char))) == NULL)
      PANIC;
  if ((old_st = malloc ((256 + 1) * sizeof (char))) == NULL)
      PANIC;
  if ((line = malloc ((20 * 256 + 1) * sizeof (char))) == NULL)
      PANIC;
  if ((dir_name = malloc ((100 + 1) * sizeof (char))) == NULL)
      PANIC;

  /* Process command line arguments and load_store if applicable */
  if (user_args(argc,argv)==0)
    load_store ();
  else
    return(1);

  /* Initialize all arguments */
  for (i = 0; i < LINEARGS; i++)
    arg[i] = NULL;

  while (1)
    {
      /* Stop CTRL-C breaking program */
      if (sigaction (SIGINT, &act, &oact) != 0)
	PANIC;
      getcwd (dir_name, 100);

      /* Prepare to take input */
      out_prompt ();
      strcpy (old_st, "");
      guess_active = 0;
      og_size = 0;
      dir_ref = get_dir_ref (dir_name);
      i = 0;
      if ((arg[i] = (char *) malloc ((256 + 1) * sizeof (char))) == NULL)
	  PANIC;
      *arg[0] = '\0';
      rectok = get_char ((int) &ch);

      /* Repeatedly take input until RETN pressed */
      while (rectok != RETN)
	{

	  switch (rectok)
	    {
	    case (FAIL):  /* Illegal Character */
	      break;
	    case (QUIT):  /* Quit program */
	      printf ("\nSaving Store.\n");
	      save_store ();
	      return (0);
	    case (EDIT):  /* Remove last character or prediction if active */
	      if (guess_active == 1)
		{
		  strcpy (arg[i], old_st);
		  guess_active = 0;
		}
	      else
		{
		  case_edit (&i);
		  strcpy (old_st, arg[i]);
		  if (strcmp ("", arg[0]) != 0)
		    if (i == 0)
		      guess_active = (call_pred_com ());
		    else
		      guess_active = (call_pred_arg (i));
		}
	      break;
	    case (NORM):  /* Append character and make prediction */
	      if (guess_active == 1)
		{
		  strcpy (arg[i], old_st);
		  guess_active = 0;
		}
	      *st = ch;
	      *(st + 1) = '\0';
	      strcat (arg[i], st);
	      strcpy (old_st, arg[i]);
	      if (i == 0)
		guess_active = (call_pred_com ());
	      else
		guess_active = (call_pred_arg (i));
	      break;
	    case (HIST):  /* Substitute history for command line */
/*** CTRL-P case first ***/
	      if ((ch == '\20') && (histcount < (ht_capacity - 1)))
		{
		  histcount++;
		  retr_hist (histcount, &i);
		}
	      else
/*** CTRL-N case now ***/
	      if ((ch == '\16') && (histcount >= 0))
		{
		  histcount--;
		  retr_hist (histcount, &i);
		}
	      break;
	    case (SPAC):  /* Space pressed (remove guess) and move to next arg */
	      if (guess_active == 1)
		strcpy (arg[i], old_st);
	    case (TABB):  /* Tab pressed move to next arg */
	      if (guess_active == 0 && i == 0)
		com_ref = in_com_list (arg[0]);
	      guess_active = 0;
	      (i < LINEARGS) ? i++ : printf ("\7");
	      og_size = 0;
	      if ((arg[i] = malloc ((30 + 1) * sizeof (char)))
		  == NULL)
		  PANIC;
	      *arg[i] = '\0';
	    }
	  /* Make an output line for display */
	  strcpy (line, arg[0]);
	  for (j = 1; j <= i; j++)
	    {
	      strcat (line, " ");
	      strcat (line, arg[j]);
	    }
	  out_line (line);
          /* Get the next character form the user */
	  rectok = get_char ((int) &ch);
	}

      histcount = -1;

      /* Prepare input mode and turn CTRL-C on ready to run command */
      set_to_run ();
      if (sigaction (SIGINT, &oact, &act) != 0)
	PANIC;
      printf ("\n");

      /* Remove any null arguments  */
      for (count = i; count >= 0; count--)
	if ((strcmp (arg[count], "")) == 0)
	  {
	    free (arg[count]);
	    for (j = count; j < i; j++)
	      arg[j] = arg[j + 1];
	    i--;
	  }
      arg[i + 1] = NULL;

      /* Is it !@ or not? Execute the command */
      if (i != -1)
	{
	  if (strncmp (arg[0], "!@", 2) == 0)
	    {
	      if (shell_commands (arg, i) != 0)
		printf
		  ("%s: Illegal shell command. Type !@he for help.\n"
		   ,argv[0]);
	    }
	  else if ((j = run_command (arg)) == 0)
	    update (arg, dir_ref);
	  add_hist ();

	  /* Prepare the input strings for another command */
	  for (j = 0; j <= i; j++)
	    {
	      free (arg[j]);
	      arg[j] = NULL;
	    }
	}

    }
}
