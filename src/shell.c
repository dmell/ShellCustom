#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <time.h>
#include "shellutil.h"

int main(int argc, char **argv)
{
	// initialization of the global variables
	logOutLen = 0;
	logErrLen = 0;
	outfile = NULL;  // the name of log outfile
	errfile = NULL;  // the name of log errfile
	// length set to -1 to check if the user has specified a new length, otherwise 4096
	logfileLenght = -1;
	bufLenght = -1;
	code = 0;  // usato come bool per opzione codice uscita

	checkParameters(argc, argv);

	FILE * fd[2];
	fd[0] = fopen(outfile, "w");
	fd[1] = fopen(errfile, "w");

	if (fd[0] == NULL || fd[1] == NULL)
	{
		perror("fopen");
		exit(1);
	}

	char *line = NULL;  // stringa dove viene memorizzato il comando inserito dall'utente
	size_t len = 0;  // ???
	ssize_t read = 0;  // numero di caratteri letti (valore di ritorno di getlineq)

	char ** cmd; // in every position there is a string containing a command and its arguments.
	int cmds=1;  // there's always one command

    //Brief description of the Shell
    printf("Sfssfsfsfsffssstackframe's Custom shell\n");
    printf("v0.9 (May 22 2018)\n");
    printf("Simple shell based on BASH with real time on file logging capabilities\n");
    printf("Type exit to dismiss\n");

	while (1) {
		fprintf(stdout, ">> ");
		fflush(stdout);
		read = getline(&line, &len, stdin);

		// remove empty spaces before the command
		int i=0;
		while (line[i] == ' ')
		{
			i++;
		}
		// NB: substring allocates dynamically the new string, so we are occupying memory twice
		line = substring(line, i, read);

		// handling of empty line and exit command
		if (strcmp(line, "\n") == 0)  // avoid to execute null command in case of empty line
		{
			continue;
		}
		if (strcmp("exit\n",line) == 0)
		{
			break;
		}

		// multiple commands
		// save here the sequence of the operators ; && ||. NB the last is an 'e' char to indicate the end
		char * operators = malloc(MAXOPERATORS*sizeof(char));
		bzero(operators, MAXOPERATORS);
		char ** commands;  // save here the commands divided by ; && ||
		commands = findMultipleCommands(&operators, line);

		int indexMultipleCommands = 0;
		int valuePreviousCommand = 0;  // used as boolean
		do {
			//strcpy(line, commands[indexMultipleCommands]);
			if ((indexMultipleCommands == 0) ||  // the first command is always executed
			   ((operators[indexMultipleCommands-1] == '|') && (valuePreviousCommand == 0)) ||  // previous command is false and this command follows a || operator
			   ((operators[indexMultipleCommands-1] == '&') && (valuePreviousCommand == 1)) ||  // previous command is true and this command follows a && operator
			   (operators[indexMultipleCommands-1] == ';'))  // this command follows a ; operator
			{
				// redirezionamento
				int out = 0; // used as boolean to check if there's a < or a > character
				int doubleChar = 0; // used as boolean to check if there's >/< or >>/<<
				// find < or >, return the filename and delete it from the command
				char * redirectFileName = redirect(&commands[indexMultipleCommands], &out, &doubleChar);
				//printf("Parsing redirezionamento: %s\n", commands[0]);
				int in_restore, out_restore, err_restore;
				int redirectFdOut, redirectFdIn;
				if (redirectFileName != NULL)
				{
					if (out == 1)
					{
					 	out_restore = dup(1);
						err_restore = dup(2);
						if (doubleChar == 0)  // a single >
						{
							redirectFdOut = open(redirectFileName, O_WRONLY | O_TRUNC | O_CREAT, 0777);
						}
						else // a double >>
						{
							redirectFdOut = open(redirectFileName, O_WRONLY | O_APPEND | O_CREAT, 0777);
						}
						dup2(redirectFdOut,1);
						dup2(redirectFdOut,2);
					}
					else
					{
						in_restore = dup(0);
						redirectFdIn = open(redirectFileName, O_RDONLY, 0777);
						if (redirectFdIn == -1)  // TODO support for << command
						{
							fprintf(stderr, "shell: the file %s does not exist!\n", redirectFileName);
							continue;
						}
						dup2(redirectFdIn,0);
					}
				}

				cmd = parseCommand(commands[indexMultipleCommands], &cmds);
				//printf("Parsing pipe: %s\n", cmd[0]);
				valuePreviousCommand = run(cmd, cmds, fd);

				for (int i = 0; i < cmds; i++)  // parseCommand allocates every time a new char **, we can free the memory
				{
					free(cmd[i]);
				}
				free(cmd);

				cmds = 1;

				if (redirectFileName != NULL)  // restore the normal stdin and stdout
				{
					if (out == 1)
					{
						dup2(out_restore,1);
						dup2(err_restore,2);
						close(redirectFdOut);
					}
					else
					{
						dup2(in_restore,0);
						close(redirectFdIn);
					}
				}
			}
			indexMultipleCommands++;
		} while (operators[indexMultipleCommands-1] != 'e');
		free(operators);
		for(int i = 0; i < indexMultipleCommands; i++)
		{
			free(commands[i]);
		}
		free(commands);
	}

	// free dynamic allocation of strings
	free(line);
	free(outfile);
	free(errfile);

	if (fclose(fd[0]) && fclose(fd[1]))
	{
		perror("fclose");
		exit(1);
	}
	return 0;
}
