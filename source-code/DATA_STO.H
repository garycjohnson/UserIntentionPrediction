extern short arg_tt_cap;

extern char* filename;

extern short load_store();

extern void save_store();

extern short search_com_match(char*,short*);

extern short get_dir_ref(char *);

extern struct prob_group get_com_probs(short,short);

extern struct prob_group get_com_tot_freq(short);

extern char* get_com_name(short);

extern short get_arg_prob(short,short);

extern char* get_arg_name(short,short);

extern short get_arg_tot_freq(short);

extern short search_arg_match(char*,short,short*);

extern void update(char**,short);

extern short in_com_list(char *);

extern short in_arg_tt(char *);

extern char* get_arg_tt(short);

extern short rem_command(char*);
	
extern short rem_argument(char*,char*);

extern short change_prob_com(char*,char*,short);
