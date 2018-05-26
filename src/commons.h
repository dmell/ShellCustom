/*

file with libraries and global variables.
Here's the declaration of substring function, used in various point of the commands

*/


#ifndef commons_h
#define commons_h

/*Libraries inclusions*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>

/* costants for the programm */
#define STDOUT 1
#define MAXOPERATORS 50
#define CMDSIZE 32 //parsers
#define DEFAULTLOGLEN 4096
#define MINLOGLEN 512
#define MAXBUF 4096
#define SEPARATOR "\n\n------------------------------------------------\n\n"
#define DATESIZE 32
#define LOGLAYOUT_DIM 4400
#define MAXLENGHT_FILENAME 256
#define READ 0
#define WRITE 1

/* global variables */
int logOutLen; // logs' current dimension
int logErrLen;
char * outfile; // logs' names
char * errfile;
int logfileLenght; // logs' lenght
int bufLenght; // buffer's lenght
int code; // flag to indicate if the user wants the return code of commands

// for stdin, stdout, stderr backup
int stdin_restore;
int stdout_restore;
int stderr_restore;

pid_t pid; // current command process pid
int killed; // set to 0 only if the process has been killed by the user

/*function definitions*/
/* extracts a substring between given indexes */
char * substring (const char * src, int first, int last);

#endif
