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

	// if man has been requested
	int man = 0; // used as bool
	for (int i = 1; i < argc; i++)
	{
		if (strcmp("--help",argv[i]) == 0)
		{
			man = 1;
		}
	}
	if (man)
	{
		showManual();
	}

	// parameters
	char *outfile = NULL;  // the name of log outfile
	char *errfile = NULL;  // the name of log errfile
	// length set to -1 to check if the user has specified a new length, otherwise 4096
	int logfileLenght = -1;
	int bufLenght = -1;
	int code = 0;  // usato come bool per opzione codice uscita
	int shortFlag; // we will use this to handle the double option to gives args
	for (int i = 1; i < argc; i++)
	{
		if (strncmp(argv[i], "-o=", 3) == 0 || strncmp(argv[i], "--outfile=", 10) == 0)  // outfile
		{
			shortFlag = strncmp(argv[i], "--outfile=", 10);
			if (outfile == NULL)  // not set yet
			{
				if (shortFlag)
				{
					outfile = estrai(argv[i], 3);
				}
				else
				{
					outfile = estrai(argv[i], 10);
				}
			}
			else  // already set, error
			{
				printf("shell: outfile parameter already entered.\n");
				printf("Try './shell --help' for more information.\n");
				exit(1);
			}
		}
		else if (strncmp(argv[i], "-e=", 3) == 0 || strncmp(argv[i], "--errfile=", 10) == 0)  // errfile
		{
			shortFlag = strncmp(argv[i], "--errfile=", 10);
			if (errfile == NULL)  // not set yet
			{
				if (shortFlag)
				{
					errfile = estrai (argv[i], 3);
				}
				else
				{
					errfile = estrai (argv[i], 10);
				}
			}
			else  // already set, error
			{
				printf("shell: errfile parameter already entered.\n");
				printf("Try './shell --help' for more information.\n");
				exit(1);
			}
		}
		else if (strncmp(argv[i], "-m=", 3) == 0 || strncmp(argv[i], "--maxlen=", 9) == 0)  // maxlen
		{
			shortFlag = strncmp(argv[i], "--maxlen=", 9);
			if (logfileLenght == -1)  // not set yet
			{
				if (shortFlag)
				{
					logfileLenght = atoi(estrai (argv[i], 3));
				}
				else
				{
					logfileLenght = atoi(estrai (argv[i], 9));
				}
			}
			else  // already set, error
			{
				printf("shell: maxlen parameter already entered.\n");
				printf("Try './shell --help' for more information.\n");
				exit(1);
			}
		}
		else if (strncmp(argv[i], "-c", 2) == 0 || strncmp(argv[i], "--code", 6) == 0)
		{
			code = 1; // the flag is set to include return code of the commands
			// N.B.: you can do it multiple times
		}
		else if (strncmp(argv[i], "-s=", 3) == 0 || strncmp(argv[i], "--size=", 7) == 0)  // buffer lenght
		{
			shortFlag = strncmp(argv[i], "--size=", 7);
			if (bufLenght == -1)  // not set yet
			{
				if (shortFlag)
				{
					bufLenght = atoi(estrai (argv[i], 3)) + 1; // to hold '\0' at the end
				}
				else
				{
					bufLenght = atoi(estrai (argv[i], 7)) + 1;
				}
			}
			else  // already set, error
			{
				printf("shell: buffer length parameter already entered.\n");
				printf("Try './shell --help' for more information.\n");
				exit(1);
			}
		}
	}

	if (logfileLenght == -1)  // if the maximum length has not been specified
		logfileLenght = DEFAULTLOGLEN;  // default

	if (bufLenght == -1)
		bufLenght = MAXBUF;

	if (logfileLenght < MINLOGLEN || logfileLenght < bufLenght)
	{
		printf("shell: error in buffer or file size.\n");
		printf("Try './shell --help' for more information.\n");
		exit(1);
	}

	// we check that the user has specified the log files
	if (outfile == NULL || errfile == NULL)
	{
		printf("shell: missing mandatory parameter.\n");
		printf("Try './shell --help' for more information.\n");
		exit(1);
	}

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
		cmd = parseCommand(line, &cmds);
		pid_t pid = fork();
		if (pid==0)
		{
			run(cmd, cmds, outfile, errfile, fd, code, bufLenght, logfileLenght);
			exit(0);
		}
		else
		{
			wait(NULL);
			cmds = 1;
		}
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
