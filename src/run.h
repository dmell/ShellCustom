/*

header file for run.c files

*/


#ifndef run_h
#define run_h

/* definitions of the functions */
/* execute the commands */
int run (char ** cmd, const int cmds, FILE ** fd);
/*handler for the SIGKILL*/
void handler (int sig);



#endif
