/*

functions for the parsing of the commands. These functions look for multiple
commands, commands divided by piping, parameters of the commands

*/


#include "commons.h"
#include "parsers.h"

// returns an array of commands divided by pipe characters
// returns the number of commands found with cmds passed by reference
char** parseCommand (const char * cmd, int * cmds)
{
	// counting commands by searching pipes
	int index = 0;
	do {
		if (cmd[index] == '|')
		{
			(*cmds)++;
		}
		index++;
	}
	while(cmd[index] != '\0');


	// at this point we create a new vector of strings, that contains a command per position
	// since we have counted how many they are, there here's the dynamic allocation
	char ** cleancmd = malloc((*cmds)*(sizeof(char *)));
	int i;
	for (i = 0; i < *cmds; i++)
	{
		cleancmd[i] = (char *)malloc(CMDSIZE+1);
	}

	// fill the array of strings
	int beg = 0; // beginning of next command
	int j = 0; // counter in array of commands
	for (i = 0; cmd[i] !='\0'; i++)
	{
		if (cmd[i] == '|')
		{
			cleancmd[j] = substring(cmd, beg, i-1);
			j++;
			beg = i + 1;
		}
	}

	// insert last command
	if (cmd[i-1] == '\n')
		cleancmd[j] = substring(cmd, beg, i-2);
	else
		cleancmd[j] = substring(cmd, beg, i-1);  // this case comes up with ||

	return cleancmd;
}


// return an array of strings with a command and its parameters
char ** splitArgs (const char * cmd)
{
	// tmp is used because after this function the string changes
	char * tmp = malloc(strlen(cmd));
	strcpy(tmp, cmd);
	char * nextToken = strtok(tmp, " ");

	char ** cleancmd = malloc((CMDSIZE/2)*(sizeof(char *)));
	int i;
	for (i = 0; i < CMDSIZE/2; i++)
	{
		cleancmd[i] = (char *)malloc(CMDSIZE+1);
	}
	i = 0;

	// fill the array of strings
	while (nextToken != NULL)
	{
		strcpy(cleancmd[i],nextToken);
		nextToken = strtok(NULL, " ");
		i++;
	}
	cleancmd[i] = NULL;  // execvp wants a NULL string at the end

	free(tmp);

	return cleancmd;
}

// return an array of commands divided by && || ;
// also fill operators, passed by reference, with the correct sequence of & | ;
char ** findMultipleCommands(char ** operators, const char * line)
{
	int counter = 0;
	int i;
	// fill operators with & | ;
	for (i=0; i<strlen(line); i++)
	{
		if (line[i] == ';')
		{
			(*operators)[counter] = ';';
			counter++;
		}
		else if ((line[i] == '|') && (line[i+1] == '|'))
		{
			(*operators)[counter] = '|';
			counter++;
			i++;  // we've checked 2 characters of line
		}
		else if ((line[i] == '&') && (line[i+1] == '&'))
		{
			(*operators)[counter] = '&';
			counter++;
			i++;  // we've checked 2 characters of line
		}
	}
	(*operators)[counter] = 'e';  // to indicate the end of the array

	char ** multipleCommands = (char **)malloc((counter+1)*(sizeof(char*)));

	counter=0;
	int first=0;
	// fill multipleCommands with the different commands
	for (i=0; i<strlen(line); i++)
	{
		if (line[i] == ';')
		{
			multipleCommands[counter] = substring(line, first, i-1);
			first=i+1;
			counter++;
		}
		else if ((line[i] == '|') && (line[i+1] == '|'))
		{
			multipleCommands[counter] = substring(line, first, i-1);
			first=i+2;
			counter++;
		}
		else if ((line[i] == '&') && (line[i+1] == '&'))
		{
			multipleCommands[counter] = substring(line, first, i-1);
			first=i+2;
			counter++;
		}
	}
	
	multipleCommands[counter] = substring(line, first, strlen(line));  // add the last command

	return multipleCommands;
}
