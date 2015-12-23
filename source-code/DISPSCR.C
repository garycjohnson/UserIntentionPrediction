#define _POSIX_SOURCE 1
#include <stdio.h>

#define DEFAULT_FILENAME ".lashdata"

#include"../Panic/panic.h"
#include"../Data/data_store.c"

void 
name_lists ()
{
  short count;

  printf ("COMMAND NAME LIST\n");
  for (count = 0; count < cl_capacity; count++)
    printf ("%hd: %s\t", count, com_list[count]);
  printf ("\n\nDIRECTORY NAME LIST\n");
  for (count = 0; count < dl_capacity; count++)
    printf ("%hd: %s\t", count, dir_list[count]);
  printf ("\n");
}

void 
arg_lists ()
{
  short count1, count2;

  printf ("ARGUMENT PROBABILITY LISTS\n");
  for (count1 = 0; count1 < cl_capacity; count1++)
    {
      printf ("_%s_\t", com_list[count1]);
      for (count2 = 0; count2 < afl_table[count1].cap; count2++)
	printf ("(\"%s\",%hd)\t", afl_table[count1].args[count2].name, afl_table[count1].args[count2].freq);
      printf ("\n");
    }
}

void 
com_table ()
{
  short count1, count2;

  printf ("\nCOMMAND-COMMAND PROBABILTY\n\t\t\tCOMMAND BEING TYPED\nL_C\t");
  for (count1 = 0; count1 < cl_capacity; count1++)
    printf ("_%hd_\t", count1);
  printf ("OCP\n");

  for (count1 = 0; count1 < cl_capacity; count1++)
    {
      printf ("_%hd_\t", count1);
      for (count2 = 0; count2 < cl_capacity; count2++)
	printf ("%hd\t", cp_table[count2][count1]);
      printf ("%hd\n", cp_table[count1][FREQ_POS]);
    }
}

void 
dir_table ()
{
  short count1, count2;

  printf ("\nDIRECTORY-COMMAND PROBABILTY\n\t\t\tCOMMAND BEING TYPED\nDIR\t");
  for (count1 = 0; count1 < cl_capacity; count1++)
    printf ("_%hd_\t", count1);

  for (count1 = 0; count1 < dl_capacity; count1++)
    {
      printf ("\n_%hd_\t", count1);
      for (count2 = 0; count2 < cl_capacity; count2++)
	printf ("%hd\t", cp_table[count2][count1 + NUM_COMS]);
    }
  printf ("\n");
}

main (int argc, char *argv[])
{
  char *choice, *old_choice;
  char *old_st;


  choice = malloc ((4 + 1) * sizeof (char));
  old_choice = malloc ((4 + 1) * sizeof (char));

  if (argc == 1)
    {
      if ((filename = malloc ((256 + 1) * sizeof (char))) == NULL)
	  PANIC;
      strcpy (filename, (char *) getenv ("HOME"));
      strcat (filename, "/");
      strcat (filename, DEFAULT_FILENAME);
    }
  else if ((argc == 3) && (strcmp (argv[1], "-f") == 0))
    if (argv[2][0] == '/')
      {
	if ((filename = malloc ((1 + strlen (argv[2])) * sizeof (char)))
	    == NULL)
	    PANIC;
	strcpy (filename, argv[2]);
      }
    else
      {
	if ((old_st = malloc ((256 + 1) * sizeof (char))) == NULL)
	    PANIC;
	getcwd (old_st, 256);
	strcat (old_st, "/");
	strcat (old_st, argv[2]);
	if ((filename = malloc ((1 + strlen (old_st)) * sizeof (char)))

	    == NULL)
	    PANIC;
	strcpy (filename, old_st);
      }
  else
    {
      printf ("Whoops!  Illegal Line Structure.\n");
      return (1);
    }

/*** END OF FILENAME CREATION ***/

  printf ("Please enter the display function required.\n(n)ames  (a)rguments  (c)ommand table  (d)irectory table\n");
  gets (choice);


  while (1)
    {
      load_store ();

      if (strchr (choice, 'n') != NULL)
	name_lists ();

      if (strchr (choice, 'a') != NULL)
	arg_lists ();

      if (strchr (choice, 'c') != NULL)
	com_table ();

      if (strchr (choice, 'd') != NULL)
	dir_table ();

      printf ("\nHit <RETURN> to rescan store.\n");
      gets (old_choice);
/*** If a new layout selected change choice ***/
      if (strcmp (old_choice, "") != 0)
	strcpy (choice, old_choice);
    }
}
