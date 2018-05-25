#ifndef parsers_h
#define parsers_h

/* definitions of the functions */
/* handles the commands */
char** parseCommand (char * cmd, int * cmds);
/* divides the commands */
char ** splitArgs (const char * cmd);
/* find multiple commands divided by ; && || */
char ** findMultipleCommands(char ** operators, char * line);

#endif
