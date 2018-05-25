#ifndef shellutil_h
#define shellutil_h

/* definitions of the functions */
/* search a redirect char */
char * redirect(char ** line, int * out, int * doubleChar);
/* estract the various options from the parameters */
void checkParameters(int argc, char ** argv);
/* show the manual in case of --help option */
void showManual();
/* when the log file dimension exceeds*/
char * dimension(FILE * fd, int *);
/* custom exit function */
void cExit(int code);

#endif
