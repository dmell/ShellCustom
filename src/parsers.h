/*

header file for parsers.c files

*/


#ifndef parsers_h
#define parsers_h


/* definitions of the functions */

/* handles the commands divided by pipe character */
char** parseCommand (const char * cmd, int * cmds);
/* divides the commands and its arguments */
char ** splitArgs (const char * cmd);
/* find multiple commands divided by ; && || */
char ** findMultipleCommands(char ** operators, const char * line);



#endif
