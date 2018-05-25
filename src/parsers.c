#include "commons.h"
#include "parsers.h"

char** parseCommand (char * cmd, int * cmds)
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
	// since we have counted how many they are, there here's the dynaminc allocation
	// (hopefully working without SEGFAULT)
	char ** cleancmd = malloc((*cmds)*(sizeof(char *)));
	int i;
	for (i = 0; i < *cmds; i++)
	{
		cleancmd[i] = (char *)malloc(CMDSIZE+1);
	}
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
	if (cmd[i-1] == '\n')
		cleancmd[j] = substring(cmd, beg, i-2);  // insert last command
	else
		cleancmd[j] = substring(cmd, beg, i-1);  // this case comes up with ||
	return cleancmd;
}

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
	while (nextToken != NULL)
	{
		strcpy(cleancmd[i],nextToken);
		nextToken = strtok(NULL, " ");
		i++;
	}
	cleancmd[i] = NULL;
	free(tmp);
	return cleancmd;
}

char ** findMultipleCommands(char ** operators, char * line)
{
	int counter = 0;
	int i;
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
	/*for (i = 0; i < counter+1; i++)
	{
		multipleCommands[i] = (char*)malloc((CMDSIZE+1)*(sizeof(char)));
	}*/

	counter=0;
	int first=0;
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
			//i++;  // we've checked 2 characters of line
			first=i+2;
			counter++;
		}
		else if ((line[i] == '&') && (line[i+1] == '&'))
		{
			multipleCommands[counter] = substring(line, first, i-1);
			//i++;  // we've checked 2 characters of line
			first=i+2;
			counter++;
		}
	}
	multipleCommands[counter] = substring(line, first, strlen(line));  // add the last command

	return multipleCommands;
}
