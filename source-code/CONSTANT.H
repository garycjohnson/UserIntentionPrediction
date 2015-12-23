

			/*** DATA_STORE.C ***/

			/*** Definitions ***/

#define NUM_COMS 40	/*** Number of commands held in table ***/
#define NUM_DIRS 20	/***           directories	      ***/
#define NUM_ARGS 8	/***           arguments per command  ***/
#define TT_SIZE 10	/*** Size of the temporal table	***/
#define FREQ_POS (NUM_COMS+NUM_DIRS) /*** Array ref for Overall Probs ***/
#define AC_MULT 0.05	/*** New command automatic probability level ***/
#define MAX_AVG	15	/*** Maximum mean ocp of commands over cl_cap ***/

#define WEIGHT_0 5
#define WEIGHT_1 2
#define WEIGHT_2 4
#define WEIGHT_3 1

			/*** Structures ***/

/*** Structure used for return of all probabilities for command matched ***/
struct prob_group {
	short ocp;
	short ccp;
	short dcp;
	short itt;
};

		/*** END OF DATA_STORE.C SECTION ***/
