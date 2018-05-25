#ifndef shellutil_h
#define shellutil_h

/* costants for the programm */
#define STDOUT 1
#define MAXOPERATORS 50
#define CMDSIZE 32
#define DEFAULTLOGLEN 4096
#define MINLOGLEN 512
#define MAXBUF 4096
#define SEPARATOR "\n\n------------------------------------------------\n\n"
#define DATESIZE 32
#define LOGLAYOUT_DIM 4400
#define MAXLENGHT_FILENAME 256

/* global variables */
int logOutLen; // logs' current dimension
int logErrLen;
char * outfile; // logs' names
char * errfile;
int logfileLenght; // logs' lenght
int bufLenght; // buffer's lenght
int code; // flag to indicate if the user wants the return code of commands
int stdin_restore;
int stdout_restore;
int stderr_restore;
pid_t pid; // command process pid
int killed; // true only if the process has been killed by the user

/* definitions of the functions */

/* find multiple commands divided by ; && || */
char ** findMultipleCommands(char ** operators, char * line);
/* search a redirect char */
char * redirect(char ** line, int * out, int * doubleChar);
/* estract the various options from the parameters */
void checkParameters(int argc, char ** argv);
/* show the manual in case of --help option */
void showManual();
/* extracts a substring between given indexes */
char * substring (char * src, int first, int last);
/* handles the commands */
char** parseCommand (char * cmd, int * cmds);
/* divides the commands */
char ** splitArgs (const char * cmd);
/* execute the commands */
int run (char ** cmd, const int cmds, FILE ** fd);
/* when the log file dimension exceeds*/
char * dimension(FILE * fd, int *);
/* custom exit function */
void cExit(int code);
void handler (int sig);

#endif
