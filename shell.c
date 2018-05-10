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
		printf("Usage: ./shell [PARAMETERS]\n");
		printf("Run the Custom Shell\n\n");
		printf("Mandatory parameters:\n");
		printf("  -o[=\"NAMEFILE\"], --outfile[=\"NAMEFILE\"]\tNAMEFILE is the log file for the stdout\n\n");
		printf("  -e[=\"NAMEFILE\"], --errfile[=\"NAMEFILE\"]\tNAMEFILE is the log file for the stderr\n\n");
		printf("Optional parameters:\n");
		printf("  -m[=NUMBER], --maxlen[=NUMBER]\t\tNUMBER is the maximum length of log files");
		printf("\n\t\t\t\t\t\t(in number of characters) (%d by default)\n\n", DEFAULTLOGLEN);
		printf("  -c,--code\t\t\t\t\talso indicates the return code of the commands\n\n");
		printf("  -s[=NUMBER],--size[=NUMBER]\t\t\tNUMBER is the maximum length of command response");
		printf("\n\t\t\t\t\t\t(in number of characters) (%d by default)\n", MAXBUF);
		exit(0);
	}

	// parameters
	char *outfile = NULL;  // the name of log outfile
	char *errfile = NULL;  // the name of log errfile
	// length set to -1 to check if the user has specified a new length, otherwise 4096
	int logfileLenght = -1;
	int bufLenght = -1;
	int code = 0;  // usato come bool per opzione codice uscita
	for (int i = 1; i < argc; i++)
	{
		if (strncmp(argv[i], "-o=", 3) == 0)  // short outfile
		{
			if (outfile == NULL)  // not set yet
			{
				outfile = estrai(argv[i], 3);
			}
			else  // already set, error
			{
				printf("shell: outfile parameter already entered.\n");
				printf("Try './shell --help' for more information.\n");
				exit(1);
			}
		}
		else if (strncmp(argv[i], "--outfile=", 10) == 0)  // long outfile
		{
			if (outfile == NULL)  // not set yet
			{
				outfile = estrai (argv[i], 10);
			}
			else  // already sey, error
			{
				printf("shell: outfile parameter already entered.\n");
				printf("Try './shell --help' for more information.\n");
				exit(1);
			}
		}
		else if (strncmp(argv[i], "-e=", 3) == 0)  // short errfile
		{
			if (errfile == NULL)  // not set yet
			{
				errfile = estrai (argv[i], 3);
			}
			else  // already sey, error
			{
				printf("shell: errfile parameter already entered.\n");
				printf("Try './shell --help' for more information.\n");
				exit(1);
			}
		}
		else if (strncmp(argv[i], "--errfile=", 10) == 0)  // long errfile
		{
			if (errfile == NULL)  // not set yet
			{
				errfile = estrai (argv[i], 10);
			}
			else  // already sey, error
			{
				printf("shell: errfile parameter already entered.\n");
				printf("Try './shell --help' for more information.\n");
				exit(1);
			}
		}
		else if (strncmp(argv[i], "-m=", 3) == 0)  // short maxlen
		{
			if (logfileLenght == -1)  // not set yet
			{
				logfileLenght = atoi(estrai (argv[i], 3));
			}
			else  // already set, error
			{
				printf("shell: maxlen parameter already entered.\n");
				printf("Try './shell --help' for more information.\n");
				exit(1);
			}
		}
		else if (strncmp(argv[i], "--maxlen=", 9) == 0)  // long maxlen
		{
			if (errfile == NULL)  // not set yet
			{
				logfileLenght = atoi(estrai (argv[i], 9));
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
		else if (strncmp(argv[i], "-s=", 3) == 0)  // short buffer lenght
		{
			if (bufLenght == -1)  // not set yet
			{
				bufLenght = atoi(estrai (argv[i], 3)) + 1; // to hold '\0' at the end
			}
			else  // already set, error
			{
				printf("shell: buffer length parameter already entered.\n");
				printf("Try './shell --help' for more information.\n");
				exit(1);
			}
		}
		else if (strncmp(argv[i], "--size=", 7) == 0)  // long buffer lenght
		{
			if (bufLenght == -1)  // not set yet
			{
				bufLenght = atoi(estrai (argv[i], 7)) + 1;
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

	int fd[2];
	fd[0] = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0777);
	fd[1] = open(errfile, O_WRONLY | O_CREAT | O_TRUNC, 0777);

	if (fd[0] < 0 || fd[1] < 0)
	{
		perror("Fail in opening files");
		cExit(1);
	}

	char *line = NULL;  // stringa dove viene memorizzato il comando inserito dall'utente
	size_t len = 0;  // ???
	ssize_t read = 0;  // numero di caratteri letti (valore di ritorno di getlineq)
	int error;  // codice di errore dell'esecuzione del comando (valore di ritorno di system)

    //Brief description of the Shell
    printf("Sfssfsfsfsffssstackframe's Custom shell\n");
    printf("v0.2 (May 9 2018)\n");
    printf("Simple shell based on BASH with real time on file logging capabilities\n");
    printf("Type exit to dismiss\n");

	while (1) {
		fprintf(stdout, ">> ");
		fflush(stdout);
		read = getline(&line, &len, stdin);
		int cmds=1;  // there's always one command
		char ** cmd = parseCommand(line, &cmds);
		int i;
		run(cmd[0],outfile,errfile, fd, code, bufLenght, logfileLenght);
	}

	// free dynamic allocation of strings
	free(line);
	free(outfile);
	free(errfile);
	// TODO: free cmd
	close(fd[0]);
	close(fd[1]);
	return 0;
}
