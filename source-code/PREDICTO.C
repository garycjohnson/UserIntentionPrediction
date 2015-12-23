#define _POSIX_SOURCE 1

#include<stdio.h>
#include<math.h>
#include<malloc.h>
#include"../Data/data_store.h"
#include"../Data/constants.h"
#include"../Panic/panic.h"
#include"../Product/debug_defs.h"

/*** Probability at which guesses are returned ***/
#define MFP 0.55
/*** Number of commands held in old_guess table ***/
#define OG_CAP 5
/*** Bigger makes temp table pass arguments more ***/
#define ATM 2
/*** Lower makes bad guys look really bad ***/
#define POW_VAL 1.2
/*** Shortens time for baddies to reform ***/
#define POW_MUL 1.8

extern FILE *dbf;
short og_size;
static short og_list[OG_CAP];
extern short dir_ref;


/*** Add a prediction to list of old guesses when returned ***/
void 
add_old_guess (int og)
{
  short count;
  if (og_size < OG_CAP)
  /*** Spaces available ***/
    {
      og_list[og_size] = og;
      og_size++;
    }
  else
  /*** No spaces available ***/
    {
      for (count = 1; count < OG_CAP; count++)
	og_list[count - 1] = og_list[count];
      og_list[OG_CAP - 1] = og;
    }
}

/*** Returns position within list that guess is held (or -1) ***/
short 
check_old_guess (short guess)
{
  short count;
  for (count = (og_size - 1); count >= 0; count--)
    if (og_list[count] == guess)
      return (og_size - count);
  return (-1); /* Not in the table */
}

/*** Calculate the likelihood value of the command from data given ***/
float 
calc_magic_form (struct prob_group f, struct prob_group t, short com)
{
  float tmp[4], ttt;
  short cog, tot_weight = WEIGHT_0 + WEIGHT_1 + WEIGHT_2 + WEIGHT_3;

  tmp[0] = f.ocp / (float) t.ocp;
  if (t.ccp != 0)
    tmp[1] = f.ccp / (float) t.ccp;
  else
    tmp[1] = 0;
  if (t.dcp != 0)
    tmp[2] = f.dcp / (float) t.dcp;
  else
    tmp[2] = 0;
  if (t.itt != 0)
    tmp[3] = f.itt / (float) t.itt;
  else
    tmp[3] = 0;

  ttt = (WEIGHT_0 * tmp[0] + WEIGHT_1 * tmp[1] + WEIGHT_2 * tmp[2] +
	 WEIGHT_3 * tmp[3]) / tot_weight;

  /* Change value to take into consideration previous offers */
  if ((cog = check_old_guess (com)) != -1)
    ttt = ttt * (1.0 - pow (POW_VAL, (-1 * POW_MUL * cog)));

  return (ttt);
}


/*** Predict command from so_far information ***/
short 
predict_com (char *so_far)
{
  short match_list[NUM_COMS];
  short num_match, count, best_guess = -1;
  float cmf_res, cur_biggest = 0, res_total = 0;

  struct prob_group probs;
  struct prob_group totals;

  /* Get totals for c_m_f calculation */
  totals = get_com_tot_freq (dir_ref);
  /* Find matches with so_far */
  num_match = search_com_match (so_far, match_list);
  /* Calculate likelihood for each member of match list */
  /* Keep check of best_guess so far */
  for (count = 0; count < num_match; count++)
    {
      probs = get_com_probs (match_list[count], dir_ref);
      cmf_res = calc_magic_form (probs, totals, match_list[count]);
      if (cmf_res > cur_biggest)
	{
	  cur_biggest = cmf_res;
	  best_guess = match_list[count];
	}
      res_total += cmf_res;
    }

  /* Take into consideration previous offers */
  if ((count = check_old_guess (best_guess)) != -1)
    cur_biggest = cur_biggest * (1 - pow (POW_VAL, (-1 * POW_MUL * count)));

  /* See if above threshold and return if applicable */
  if ((best_guess != -1)
      && ((cur_biggest / res_total) > MFP))
    {
      add_old_guess (best_guess);
      return (best_guess);
    }
  else
    add_old_guess (-1);

  return (-1);
}


/*** Predict argument from so_far and command information */
short 
predict_arg (char *so_far, short com)
{
  short match_list[NUM_ARGS + TT_SIZE];
  short total, count, count2, num_match, temp_top, found, cog, best_guess = -1;
  char *temp_str;
  float prob, cur_biggest = 0, res_total = 0;

  if ((temp_str = (char *) malloc ((256 + 1) * sizeof (char))) == NULL)
      PANIC;

  total = get_arg_tot_freq (com);
  /* Make list of matches to calculate likelihoods */
  num_match = search_arg_match (so_far, com, match_list);

  /* Append to match_list any entries in temp_table which also */
  /* match and are not in arg list */
  temp_top = num_match;
  for (count = 0; count < arg_tt_cap; count++)
    {
      found = 0;
      count2 = temp_top;
      temp_str = get_arg_tt (count);
      if (strncmp (so_far, temp_str, strlen (so_far)) == 0)
	{
	  /* In commands argument list */
	  if (in_arg_list (temp_str, com) != -1)
	    found = 1;
	  else
	    while (found == 0 && count2 < num_match)
	      {
		/* Already matched from temp table */
		if (strcmp (
			     get_arg_tt (match_list[count2] - NUM_ARGS),
			     temp_str) == 0)
		  found = 1;
		count2++;
	      }
	  /* First occurrence: add to match list */
	  if (found == 0)
	    {
	      match_list[num_match] = count + NUM_ARGS;
	      num_match++;
	    }
	}
    }

  /* For each value in the table calculate the likelihood value */
  /* and keep track of best guess so far */
  for (count = 0; count < num_match; count++)
    {
      if (match_list[count] < NUM_ARGS)
	{
	  prob = get_arg_prob (com, match_list[count]);
	  strcpy (temp_str, get_arg_name (com, match_list[count]));
	  prob += in_arg_tt (temp_str) * ATM;
	  if ((cog = check_old_guess (match_list[count])) != -1)
	    prob = prob * (1.0 - pow (POW_VAL, (-1 * POW_MUL * cog)));
	  if (prob > cur_biggest)
	    {
	      cur_biggest = prob;
	      best_guess = match_list[count];
	    }
	  res_total += prob;
	}
      else
	{
	  strcpy (temp_str, get_arg_tt (match_list[count] - NUM_ARGS));
	  prob = in_arg_tt (temp_str) * ATM;
	  if ((cog = check_old_guess (count)) != -1)
	    prob = prob * (1.0 - pow (POW_VAL, (-1 * POW_MUL * cog)));
	  if (prob > cur_biggest)
	    {
	      cur_biggest = prob;
	      best_guess = match_list[count];
	    }
	  res_total += prob;
	}
    }

  free (temp_str);
  if ((cog = check_old_guess (best_guess)) != -1)
    cur_biggest = cur_biggest * (1.0 - pow (POW_VAL, (-1 * POW_MUL * cog)));


  /* If best guess is above threshold value return it */
  if ((best_guess != -1)
      && ((cur_biggest / res_total) > MFP))
    {
      add_old_guess (best_guess);
      return (best_guess);
    }
  else
    /* If no guess returned */
    add_old_guess (-1);

  return (-1);
}
