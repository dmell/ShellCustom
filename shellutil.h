#ifndef shellutil_h
#define shellutil_h

/* costants for the programm */
#define STDOUT 1
#define CMDSIZE 32
#define DEFAULTLOGLEN 4096
#define MINLOGLEN 512
#define MAXBUF 4096
#define SEPARATOR "\n------------------------------------------------\n\n"
#define SEPARATOR_LEN 51
#define DATESIZE 32
#define LOGLAYOUT_CODE_OUT 120
#define LOGLAYOUT_CODE_ERR 126
#define LOGLAYOUT_NOCODE_OUT 105
#define LOGLAYOUT_NOCODE_ERR 111

/* global variables */
int logOutLen;
int logErrLen;


/* definitions of the functions */
void showManual();
/* extracts filename in args */
char * estrai (char *source, int index);
/* extracts a substring between given indexes */
char * substring (char * src, int first, int last);
/* handles the commands */
char** parseCommand (char * cmd, int * cmds);
/* divides the commands */
char ** splitArgs (const char * cmd);
/* execute the commands */
void run (char * cmd, char * outfile, char * errfile, int * fd, int codeFlag, int bufLenght, int logfileLenght);
/* when the log file dimension exceeds*/
void dimension(int * fd, int *);
/* custom exit function */
void cExit(int code);

#endif
