#define _POSIX_SOURCE 1
/*******************************************************************************
/***	data_store.c	Module for LASH		Gary Johnson	1994
/***	Controls all handling of the command and argument probability tables.
/***	Heavily used by predict for data necessary for guess production.
/***	Also handles most of the !@ shell command functions
/******************************************************************************/


#include<string.h>
#include<malloc.h>
#include<stdio.h>
#include"../Panic/panic.h"

#include"constants.h"


/*** These structures define the contents of the argument frequency table ***/
struct arg_freq
  {
    char *name;
    short freq;
  };
struct arg_freq_list
  {
    struct arg_freq args[NUM_ARGS];
    short cap;
    short tot_freq;
  };

/*** File scope variable definitions : static for data hiding purposes ***/
static struct arg_freq_list afl_table[NUM_COMS];
static short cp_table[NUM_COMS + 1][FREQ_POS + 1];
static short cl_capacity;	/*Number of commands currently held (<=NUM COMS) */
static short dl_capacity;	/*Number of dirs currently held (<=NUM_DIRS) */
static short tt_capacity;	/*Number of filled elts in temp table */
short arg_tt_cap;		/*Number of filled elts in arg temp */
static char *com_list[NUM_COMS];	/*List of commands held for matching */
static char *dir_list[NUM_DIRS];	/*List of directories held for matching */
static short last_comm;		/*Reference of last command successfully executed */
static short temp_table[TT_SIZE];	/*Temporal Locality list of com refs */
static char *arg_temp[TT_SIZE];	/*Temporal Locality of arguments */
char *filename;

/*** Loads the command and argument frequency tables ***/
void 
load_store ()
{
  FILE *fp;
  char *inp_str;
  short count, count2;
  if ((inp_str = malloc ((256 + 1) * sizeof (char))) == NULL)
      PANIC;

  if ((fp = fopen (filename, "r")) == NULL)
    {
      printf ("Creating new data file: \"%s\"\n", filename);
      return;
    }

/*** Load the Com Prob Table ***/
  fread (cp_table, sizeof (short), (FREQ_POS + 1) * (NUM_COMS + 1), fp);

  fscanf (fp, "%hd ", &cl_capacity);
  fscanf (fp, "%hd ", &dl_capacity);

/*** Load in the Command List ***/
  for (count = 0; count < cl_capacity; count++)
    {
      fscanf (fp, "%s", inp_str);
      com_list[count] = malloc ((strlen (inp_str) + 1) * sizeof (char));
      strcpy (com_list[count], inp_str);
    }

/*** Load in the Directory List ***/
  for (count = 0; count < dl_capacity; count++)
    {
      fscanf (fp, "%s", inp_str);
      dir_list[count] = malloc ((strlen (inp_str) + 1) * sizeof (char));
      strcpy (dir_list[count], inp_str);
    }

/*** Load in the Argument Lists ***/
  for (count = 0; count < cl_capacity; count++)
    {
      fscanf (fp, "%hd %hd ", &afl_table[count].tot_freq,
	      &afl_table[count].cap);

      for (count2 = 0; count2 < afl_table[count].cap; count2++)
	{
	  fscanf (fp, "%s%hd ", inp_str,
		  &afl_table[count].args[count2].freq);
	  afl_table[count].args[count2].name =
	    malloc ((strlen (inp_str) + 1) * sizeof (char));
	  strcpy (afl_table[count].args[count2].name, inp_str);
	}
    }

  fclose (fp);
}

/*** Down-sizes the table values to allow easier access for new coms ***/
void 
normalize_cpt ()
{
  float change;
  short count, count2, delta, delta_tot;

  for (count = 0; count <= FREQ_POS; count++)
/*** Only normalize if mean of com probs is greater than MAX_AVG ***/
    if (cp_table[NUM_COMS][count] > (cl_capacity * MAX_AVG))
      {
	change = MAX_AVG * cl_capacity / (float) cp_table[NUM_COMS][count];
	delta_tot = 0;
	for (count2 = 0; count2 < cl_capacity; count2++)
	  {
	    delta = cp_table[count2][count] * (2 * (1 - change));
	    delta_tot += delta;
	    cp_table[count2][count] -= delta;
	  }
	cp_table[NUM_COMS][count] -= delta_tot;
      }

/*** Normalization of argument table ***/
  for (count = 0; count < cl_capacity; count++)
    if (afl_table[count].tot_freq > (afl_table[count].cap * MAX_AVG))
      {
	change = afl_table[count].cap * MAX_AVG /
	  (float) afl_table[count].tot_freq;
	delta_tot = 0;
	for (count2 = 0; count2 < afl_table[count].cap; count2++)
	  {
	    delta = (short) afl_table[count].args[count2].freq * (2 * (1 - change));
	    delta_tot += delta;
	    afl_table[count].args[count2].freq -= delta;
	  }
	afl_table[count].tot_freq -= delta_tot;
      }
}

/*** Normalizes and saves the command and argument frequency tables ***/
void 
save_store ()
{
  FILE *fp;
  short count, count2;

  normalize_cpt ();

  if ((fp = fopen (filename, "w")) == NULL)
    PANIC;
/*** Save the Command Probability Table ***/
  fwrite (cp_table, sizeof (short), (FREQ_POS + 1) * (NUM_COMS + 1), fp);

  fprintf (fp, "%hd %hd ", cl_capacity, dl_capacity);

/*** Save the Command List ***/
  for (count = 0; count < cl_capacity; count++)
    fprintf (fp, "%s\n", com_list[count]);

/*** Save the Directory List ***/
  for (count = 0; count < dl_capacity; count++)
    fprintf (fp, "%s\n", dir_list[count]);

/*** Save the Argument Lists ***/
  for (count = 0; count < cl_capacity; count++)
    {
      fprintf (fp, "%hd %hd ", afl_table[count].tot_freq,
	       afl_table[count].cap);
      for (count2 = 0; count2 < afl_table[count].cap; count2++)
	fprintf (fp, "%s %hd ",
		 afl_table[count].args[count2].name,
		 afl_table[count].args[count2].freq);
    }

  fclose (fp);
}

/*** Returns a list of command references matching the so_far string ***/
short 
search_com_match (char *so_far, short *match_list)
{
  short count, len, ret = 0;

  len = strlen (so_far);

  for (count = 0; count < cl_capacity; count++)
    if (strncmp (so_far, com_list[count], len) == 0)
    /* If com_list[count]=so_far */
      {
	match_list[ret] = count;
	ret++;
      }
  return (ret);			/*Number of matches made */
}

/*** Gives frequency from command ref * cur_dir ***/
short 
dir_com_prob (short match, short dir)
{
  short ret;
  if (dir != -1)
    ret = cp_table[match][dir + NUM_COMS];
  else
    ret = 0;

  return (ret);
}

/*** Returns the number of times a command appears in the temporal table ***/
short 
in_temp_table (short match)
{
  short ret = 0, count;

  for (count = 0; count < tt_capacity; count++)
    if (match == temp_table[count])
      ret++;
  return (ret);
}

/*** Returns the number of times an argument appears in the temporal table ***/
short 
in_arg_tt (char *match)
{
  short ret = 0, count;

  for (count = 0; count < arg_tt_cap; count++)
    if (strcmp (match, arg_temp[count]) == 0)
      ret++;
  return (ret);
}



/*** For a particular command reference returns all associated ***/
struct prob_group 
get_com_probs (short match, short dir)
{
  struct prob_group ret;

  ret.ocp = cp_table[match][FREQ_POS];
  ret.ccp = cp_table[match][last_comm];
  ret.dcp = dir_com_prob (match, dir);
  ret.itt = in_temp_table (match);

  return (ret);
}

/*** Adds a new directory to the list of held directories and resets table ***/
short 
add_new_dir (char *dir)
{
  short dir_ref, low_freq, count, len;
  low_freq = cp_table[NUM_COMS][FREQ_POS];

  len = strlen (dir);

  if (dl_capacity < NUM_DIRS)
  /* If spaces available */
    {
      if ((dir_list[dl_capacity] = 
	   (char *) malloc ((len + 1) * sizeof (char))) == NULL)
	   PANIC;
      dir_ref = dl_capacity;
      dl_capacity++;
    }
  else
  /* No spaces available */
    {
      for (count = 0; count < NUM_DIRS; count++)
	if (cp_table[NUM_COMS][NUM_COMS + count] < low_freq)
	  {
	    low_freq = cp_table[NUM_COMS][NUM_COMS + count];
	    dir_ref = count;
	  };
      if (realloc (dir_list[dir_ref], (len + 1) * sizeof (char)) == NULL)
	  PANIC;
    }
  strcpy (dir_list[dir_ref], dir);

  /* Reset data values for directory to zero */
  for (count = 0; count <= NUM_COMS; count++)
    cp_table[count][NUM_COMS + dir_ref] = 0;

  return dir_ref;
}

/*** Returns the reference for the directory adding if not in table ***/
short 
get_dir_ref (char *dir)
{
  short count = 0, ret = -1;

  while (count < dl_capacity && ret == -1)
    {
      if (strcmp (dir_list[count], dir) == 0)
	ret = count;
      count++;
    }
  if (ret == -1)
    ret = add_new_dir (dir);
  return ret;
}

/*** Gives all total_frequencies for referencing against individual frequencies ***/
struct prob_group 
get_com_tot_freq (short dir)
{
  struct prob_group ret;

  ret.ocp = cp_table[NUM_COMS][FREQ_POS];
  ret.ccp = cp_table[NUM_COMS][last_comm];
  ret.itt = tt_capacity;

  if (dir != -1)
    ret.dcp = cp_table[NUM_COMS][dir + NUM_COMS];
  else
    ret.dcp = 0;

  return (ret);
}

/*** Gives command name matched with reference ***/
char *
get_com_name (short match_ref)
{
  return (com_list[match_ref]);
}

/*** Gives frequency for command * argument ***/
short 
get_arg_prob (short com_ref, short arg_ref)
{
  return (afl_table[com_ref].args[arg_ref].freq);
}

/*** Gives argument name mathced with reference ***/
char *
get_arg_name (short com_ref, short arg_ref)
{
  return (char *) (afl_table[com_ref].args[arg_ref].name);
}

/*** Gives total frequency for referencing argument probability against ***/
short 
get_arg_tot_freq (short com_ref)
{
  return (afl_table[com_ref].tot_freq);
}

/*** Returns list of argument references matching so_far string ***/
short 
search_arg_match (char *so_far, short com_ref, short *match_list)
{
  short count, idx = 0, len;
  len = strlen (so_far);
  for (count = 0; count < afl_table[com_ref].cap; count++)

    if ((strncmp (afl_table[com_ref].args[count].name, so_far, len)) == 0)
      {
	match_list[idx] = count;
	idx++;
      }
  return (idx);
}

/*** Adds a command into the temporal table ***/
void 
add_temp_table (short match_ref)
{
  short count;
  if (tt_capacity < TT_SIZE)
  /* If spaces are available */
    {
      temp_table[tt_capacity] = match_ref;
      tt_capacity++;
    }
  else
  /* No spaces available */
    {
      for (count = 1; count < TT_SIZE; count++)
	temp_table[count - 1] = temp_table[count];
      temp_table[TT_SIZE - 1] = match_ref;
    }
}

/*** Adds a command into the temporal table ***/
void 
add_arg_tt (char *match)
{
  short count;

  if (arg_tt_cap < TT_SIZE)
  /* If spaces are available */
    {
      if ((arg_temp[arg_tt_cap] = malloc ((strlen (match) + 1) * sizeof (char)))
	  == NULL)
	  PANIC;
      strcpy (arg_temp[arg_tt_cap], match);
      arg_tt_cap++;
    }
  else
  /* If no spaces available */
    {
      free (arg_temp[0]);
      for (count = 1; count < TT_SIZE; count++)
	arg_temp[count - 1] = arg_temp[count];
      if ((arg_temp[TT_SIZE - 1] = malloc ((strlen (match) + 1) * sizeof (char)))
	  == NULL)
	  PANIC;
      strcpy (arg_temp[TT_SIZE - 1], match);
    }
}

/*** Returns a value from the argument temporal table ***/
char *
get_arg_tt (short idx)
{
  if (idx < arg_tt_cap)
    return (arg_temp[idx]);
  else
    return NULL;
}

/*** Clears old commands from the temporal table when swapped out ***/
void 
clear_temp_table (short old_ref)
{
  short count1, count2;

  for (count1 = 0; count1 < tt_capacity; count1++)
    if (temp_table[count1] == old_ref)
    /* If found compress all others down and lower tt_capacity */
      {
	for (count2 = count1 + 1; count2 < tt_capacity; count2++)
	  temp_table[count2 - 1] = temp_table[count2];
	tt_capacity--;
      };
}

/*** Adds new command to com_list and resets all table references ***/
short 
add_new_com (char *name, short dir_ref)
{
  short count, lf_ref, low_freq, len;
  low_freq = cp_table[NUM_COMS][FREQ_POS];

  len = strlen (name);

  /* Add name */
  if (cl_capacity < NUM_COMS)
  /* If spaces are available */
    {
      if ((com_list[cl_capacity] = 
          (char *) malloc ((len + 1) * sizeof (char))) == NULL)
	  PANIC;
      lf_ref = cl_capacity;
      cl_capacity++;
    }
  else
  /* No spaces available */
    {
      for (count = 0; count < NUM_COMS; count++)
	if (cp_table[count][FREQ_POS] < low_freq)
	  {
	    low_freq = cp_table[count][FREQ_POS];
	    lf_ref = count;
	  }
      if (realloc (com_list[lf_ref], (len + 1) * sizeof (char)) == NULL)
	  PANIC;
      for (count = 0; count <= FREQ_POS; count++)
	cp_table[NUM_COMS][count] =
	  cp_table[NUM_COMS][count] - cp_table[lf_ref][count];

      clear_temp_table (lf_ref);
    }
  strcpy (com_list[lf_ref], name);

  /* Sort out placement values */
  for (count = 0; count <= NUM_COMS; count++)
    {
      cp_table[lf_ref][count] = 0;
      cp_table[count][lf_ref] = 0;
    }


  if (last_comm != -1)
    {
      cp_table[lf_ref][last_comm] = (cp_table[NUM_COMS][last_comm] * AC_MULT) + 1;
      cp_table[NUM_COMS][last_comm] = cp_table[NUM_COMS][last_comm] 
							* (1 + AC_MULT) + 1;
    }

  cp_table[lf_ref][FREQ_POS] = (cp_table[NUM_COMS][FREQ_POS] * AC_MULT) + 1;
  cp_table[NUM_COMS][FREQ_POS] = (cp_table[NUM_COMS][FREQ_POS] * (1 + AC_MULT) + 1);

  cp_table[lf_ref][NUM_COMS + dir_ref] =
    (cp_table[NUM_COMS][NUM_COMS + dir_ref] * AC_MULT) + 1;

  cp_table[NUM_COMS][NUM_COMS + dir_ref] =
    (cp_table[NUM_COMS][NUM_COMS + dir_ref] * (1 + AC_MULT)) + 1;

  afl_table[lf_ref].cap = 0;
  afl_table[lf_ref].tot_freq = 0;

  return (lf_ref);
}

/*** Adds new argument to com_refs set of arguments ***/
short 
add_new_arg (short com_ref, char *arg_name)
{
  short count, low_freq, lf_ref, len;
  low_freq = (afl_table[com_ref].tot_freq);

  len = strlen (arg_name);

  if (afl_table[com_ref].cap < NUM_ARGS)
  /* If spaces are available */
    {
      lf_ref = afl_table[com_ref].cap;
      if ((afl_table[com_ref].args[lf_ref].name =
	   malloc ((len + 1) * sizeof (char))) == NULL)
	  PANIC;
      afl_table[com_ref].args[lf_ref].freq =
	afl_table[com_ref].tot_freq * 0.1 + 1;
      afl_table[com_ref].tot_freq = afl_table[com_ref].tot_freq * 1.1 + 1;
      afl_table[com_ref].cap++;

    }
  else
  /* No spaces available */
    {
      for (count = 0; count < NUM_ARGS; count++)
	if (afl_table[com_ref].args[count].freq < low_freq)
	  {
	    low_freq = afl_table[com_ref].args[count].freq;
	    lf_ref = count;
	  };
      if (realloc (afl_table[com_ref].args[lf_ref].name,
		   (len + 1) * sizeof (char)) == NULL)
	  PANIC;
      afl_table[com_ref].args[lf_ref].freq =
	(afl_table[com_ref].tot_freq * 0.1) + 1;
      afl_table[com_ref].tot_freq =
	(afl_table[com_ref].tot_freq * 1.1) - low_freq + 1;
    };
  strcpy (afl_table[com_ref].args[lf_ref].name, arg_name);
  return (lf_ref);
}

/*** Updates frequency values for an argument after it has been used ***/
void 
update_arg (short arg, short com)
{
  afl_table[com].tot_freq++;
  afl_table[com].args[arg].freq++;
}

/*** Checks to see if an argument string exists in the argument list for com ***/
short 
in_arg_list (char *arg, short com)
{
  short ret = -1, count = 0;
  while (count < afl_table[com].cap && ret == -1)
    if (strcmp (arg, afl_table[com].args[count].name) == 0)
      ret = count;
    else
      count++;
  return ret;
}

/*** Checks to see if a command string exists in the command list ***/
short 
in_com_list (char *com)
{
  short ret = -1, count = 0;

  while (count < cl_capacity && ret == -1)
    {
      if (strcmp (com, com_list[count]) == 0)
	ret = count;
      count++;
    }
  return ret;
}

/*** Checks to see if a directory exists in the directory list ***/
short 
in_dir_list (char *dir)
{
  short ret = -1, count = 0;

  while (count < dl_capacity && ret == -1)
    {
      if (strcmp (dir, dir_list[count]) == 0)
	ret = count;
      count++;
    }
  return ret;
}


/*** Removes Command from Command List ***/
short 
rem_command (char *com)
{
  short posn, count, count2;

  if ((posn = in_com_list (com)) == -1)
    {
      printf ("\"%s\" does not exist in Command Table.\n", com);
      return (1);
    }
  else
    {
      free (com_list[posn]);
      for (count = posn; count < (cl_capacity - 1); count++)
	{
	  com_list[count] = com_list[count + 1];
	  afl_table[count] = afl_table[count + 1];
	}

      clear_temp_table (posn);

      for (count = 0; count <= FREQ_POS; count++)
	cp_table[NUM_COMS][count] -= cp_table[posn][count2];
      for (count = posn; count < (cl_capacity - 1); count++)
	for (count2 = 0; count2 <= FREQ_POS; count2++)
	  cp_table[count][count2] = cp_table[count + 1][count2];
      for (count = posn; count < (cl_capacity - 1); count++)
	for (count2 = 0; count2 <= NUM_COMS; count2++)
	  cp_table[count2][count] = cp_table[count2][count + 1];
      cl_capacity--;
      return (0);
    }
}

/*** Removes Argument from Command's List ***/
short 
rem_argument (char *com, char *arg)
{
  short com_ref, arg_ref, count;

  if ((com_ref = in_com_list (com)) == -1)
    {
      printf ("\"%s\" does not exist in Command Table.\n", com);
      return (1);
    }
  else if ((arg_ref = in_arg_list (arg, com_ref)) == -1)
    {
      printf ("\"%s\" does not exist in Argument List.\n", arg);
      return (1);
    }
  else
    {
      afl_table[com_ref].tot_freq -= afl_table[com_ref].args[arg_ref].freq;
      free (afl_table[com_ref].args[arg_ref].name);
      for (count = arg_ref; count < (afl_table[com_ref].cap - 1); count++)
	afl_table[com_ref].args[count] = afl_table[com_ref].args[count + 1];
      afl_table[com_ref].cap--;
      return (0);
    }
}

/*** Removes Directory from List ***/
short 
rem_dir (char *dir)
{
  short posn, count, count2;

  if ((posn = in_dir_list (dir)) == -1)
    {
      printf ("\"%s\" does not exist in directory list.", dir);
      return (1);
    }
  else
    {
      free (dir_list[posn]);
      dl_capacity--;
      for (count = posn; count < dl_capacity; count++)
	dir_list[count] = dir_list[count + 1];
      posn += NUM_COMS;
      for (count = posn; count < (NUM_COMS + dl_capacity - 1); count++)
	cp_table[count2][count] = cp_table[count2][count + 1];
      return (0);
    }
}


/*** Increases/Decreases Probability Value for a command ***/
short 
change_prob_com (char *com, char *val, short parity)
{
  short percent = 0, change, count = 0;
  float temp;

  while (val[count] != '\0')
    {
      if (isdigit (val[count]) != 0 && (val[count] != '0'))
	percent = (percent * 10) + (val[count] - '0');
      else if ((val[count] == '0') && (count != 0))
	percent = percent * 10;
      else
	{
	  printf ("Illegal value for change.\n");
	  return (1);
	}
      count++;
    }

  if ((count = in_com_list (com)) != -1)
    if ((percent > 0) && (percent <= 100))
      {
	percent = percent * parity;
	temp = (float) percent / 100;
	change = (short) (cp_table[count][FREQ_POS] * temp);
	cp_table[count][FREQ_POS] += change;
	cp_table[NUM_COMS][FREQ_POS] += change;
	return (0);
      }
    else
      {
	printf ("Illegal value for change.\n");
	return (1);
      }
  else
    {
      printf ("\"%s\" does not exist in Command Table.\n", com);
      return (1);
    }

  return (0);
}

/*** Increases/Decreases Probability Value for an argument ***/
short 
change_prob_arg (char *com, char *arg, char *val, short polarity)
{
  short com_ref, arg_ref, percent = 0, change, count = 0;
  float temp;

  while (val[count] != '\0')
    {
      if (isdigit (val[count]) != 0 && (val[count] != '0'))
	percent = (percent * 10) + (val[count] - '0');
      else if ((val[count] == '0') && (count != 0))
	percent = percent * 10;
      else
	{
	  printf ("Illegal value for change.\n");
	  return (1);
	}
      count++;
    }

  if ((com_ref = in_com_list (com)) != -1)
    if ((arg_ref = in_arg_list (arg, com_ref)) != -1)
      if ((percent > 0) && (percent <= 100))
	{
	  percent = percent * polarity;
	  temp = (float) percent / 100;
	  change = (short) (afl_table[com_ref].args[arg_ref].freq * temp);
	  afl_table[com_ref].args[arg_ref].freq += change;
	  afl_table[com_ref].tot_freq += change;
	}
      else
	{
	  printf ("Illegal value for change.\n");
	  return (1);
	}
    else
      {
	printf ("\"%s\" is not present in argument list", arg);
	return (1);
      }
  else
    {
      printf ("\"%s\" does not exist in Command Table.\n", com);
      return (1);
    }

  return (0);
}

short 
list_coms ()
{
  short count;
  for (count = 0; count < cl_capacity; count++)
    printf ("%s\n", com_list[count]);
  return (0);
}

short 
list_dirs ()
{
  short count;
  for (count = 0; count < dl_capacity; count++)
    printf ("%s\n", dir_list[count]);
  return (0);
}

short 
list_args (char *com)
{
  short count, com_ref;
  if ((com_ref = in_com_list (com)) != -1)
    {
      for (count = 0; count < afl_table[com_ref].cap; count++)
	printf ("%s\n", afl_table[com_ref].args[count].name);
      return (0);
    }
  else
    {
      printf ("\"%s\" does not exist in Command Table.\n", com);
      return (1);
    }
}
/*** Updates the frequency values for a command after being used ***/
void 
update_com (short com, short dir)
{
  if (last_comm != -1)
    {
      cp_table[com][last_comm]++;
      cp_table[NUM_COMS][last_comm]++;
    }

  cp_table[com][FREQ_POS]++;
  cp_table[NUM_COMS][FREQ_POS]++;

  cp_table[com][NUM_COMS + dir]++;
  cp_table[NUM_COMS][NUM_COMS + dir]++;
}

/*** Called after each successful execution to update values in data store ***/
void 
update (char **execs, short dir_ref)
{

  short com_ref, arg_ref, count = 1;

  /* Update command values */
  if ((com_ref = in_com_list (execs[0])) == -1)
    com_ref = add_new_com (execs[0], dir_ref);
  else
    update_com (com_ref, dir_ref);

  /* Update argument values for every argument */
  while (execs[count] != NULL)
    {
      if ((arg_ref = in_arg_list (execs[count], com_ref)) == -1)
	arg_ref = add_new_arg (com_ref, execs[count]);
      else
	update_arg (arg_ref, com_ref);
      add_arg_tt (execs[count]);
      count++;
    }

  /* Update temp table and prepare global */
  last_comm = com_ref;
  add_temp_table (com_ref);
}
