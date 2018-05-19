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

	// parameters
	char *outfile = NULL;  // the name of log outfile
	char *errfile = NULL;  // the name of log errfile

	// length set to -1 to check if the user has specified a new length, otherwise 4096
	int logfileLenght = -1;
	int bufLenght = -1;
	int code = 0;  // usato come bool per opzione codice uscita

	checkParameters(argc, argv, &outfile, &errfile, &logfileLenght, &bufLenght, &code);

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
	int error;  // codice di errore dell'esecuzione del comando (valore di ritorno di system)

	char ** cmd; // in every position there is a string containing a command and its arguments.
	int cmds=1;  // there's always one command

    //Brief description of the Shell
    printf("Sfssfsfsfsffssstackframe's Custom shell\n");
    printf("v0.69 (May 17 2018)\n");
    printf("Simple shell based on BASH with real time on file logging capabilities\n");
    printf("Type exit to dismiss\n");

	while (1) {
		fprintf(stdout, ">> ");
		fflush(stdout);
		read = getline(&line, &len, stdin);
        //TODO: function to check if the users entered an empty line
		if (strcmp(line, "\n") == 0)  // avoid to execute null command in case of empty line
		{
			continue;
		}
		if (strcmp("exit\n",line) == 0)
		{
			break;
		}
		cmd = parseCommand(line, &cmds);
		run(cmd, cmds, fd, code, bufLenght, logfileLenght);
		cmds = 1;
	}

	// free dynamic allocation of strings
	free(line);
	free(outfile);
	free(errfile);

	for (int i = 0; i < cmds; i++)
	{
		free(cmd[i]);
	}
	free(cmd);

	if (fclose(fd[0]) && fclose(fd[1]))
	{
		perror("fclose");
		exit(1);
	}
	return 0;
}
