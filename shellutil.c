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
#define READ 0
#define WRITE 1

void showManual()
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

char** parseCommand (char * cmd, int * cmds)
{
	if (strncmp(cmd, "exit", 4) == 0)
	{
		cExit(0);
	}

	// TODO: maybe it would be cool to handle ; and && too

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
	cleancmd[j] = substring(cmd, beg, i-2);  // insert last command
	return cleancmd;
}

char * estrai (char *source, int index)
{
	char * tmp = NULL;
	tmp = malloc(1024);

	int i = index;

	while (1)
	{
		if (source[i] == '\0')
		{
			break;
		}

		tmp[i-index] = source[i];

		i++;
	}
	return tmp;
}

char * substring (char * src, int first, int last)
{
	char * res = malloc(last - first + 2);
	int i;
	int j = 0;
	for (int i = first; i <= last; i++)
	{
		res[j] = src[i];
		j++;
	}
	res[j] = '\0';
	return res;
}

void run (char * cmd, char * outfile, char * errfile, int * fd, int codeFlag, int bufLenght, int logfileLenght)
{
	pid_t pid;
	int returnCode;
    int maxOutLogLenght = logfileLenght; // Two variables to handle separately the
    int maxErrLogLenght = logfileLenght; // relative dimension of the log files
	// date and time of the execution
	time_t t = time(NULL);
	struct tm *tm = localtime(&t);

	// the idea is to make the two processes comunicate with a pipe, in order to send the
	// output of the command to the parent and make it print.
	int fdIPC_out[2];
	int fdIPC_err[2];
	pipe(fdIPC_out);
	pipe(fdIPC_err);

	// we use the function to create an array of strings
	char ** cmdSplitted = splitArgs(cmd);

	pid = fork();

	if (pid < 0)
	{
		printf("Error forking!\n");
		exit(1);
	}
	else if (pid == 0) // child
	{
		//	printf("Command %s, child process\n", cmd);
		close(fdIPC_out[READ]); // child doesn't read
		close(fdIPC_err[READ]);
		dup2(fdIPC_out[WRITE], 1);
		dup2(fdIPC_err[WRITE], 2);
		int execError = execvp(cmdSplitted[0], cmdSplitted);
		int commandError = errno;  // NB errno is not setted as the error of the command
								   // but is setted as a symbolic error based on the error type of the execvp
		// entra sempre qui e la stringa è sbagliata, c'è qualcosa che non torna
		if (execError == -1)
		{
			fprintf(stderr,"%s: command not found\n",cmdSplitted[0]);
			exit(commandError);
		}
	}
	else // parent
	{
		//printf("Back to the parent\n");
		int returnCode;
		wait(&returnCode);
		returnCode = WEXITSTATUS(returnCode);
		close(fdIPC_out[WRITE]); // parent doesn't write
		close(fdIPC_err[WRITE]);

		//INITIALIZING BUFFERS
		char buf[MAXBUF]; // stdout buff
		char buf2[MAXBUF]; // stderr buff
		char date[DATESIZE];
		bzero(buf, MAXBUF); // clean the stdout buffer
		bzero(buf2, MAXBUF); // clean the stderr buffer
		bzero(date, DATESIZE); //clean the date buffer

		int dim = read(fdIPC_out[READ], buf, MAXBUF);  // read the output written from the child in pipe
		int dim2 = read(fdIPC_err[READ], buf2, MAXBUF);

		// close file descriptors, we don't need to communicate with the child anymore
		close(fdIPC_out[READ]);
		close(fdIPC_err[READ]);

		// N.B.: we read MAXBUF character and then we will check if it is less then bufLenght
		if (dim > bufLenght)
		{
			printf("The output of the command is too long.\n\n");
			return;
		}
		int dimOutNewCmd = strlen(cmd) + dim; // we want to print dim characters as the output of the cmd
		int dimErrNewCmd = strlen(cmd) + dim2;
		if (codeFlag == 1)
		{
			dimOutNewCmd += LOGLAYOUT_CODE_OUT;
			dimErrNewCmd += LOGLAYOUT_CODE_ERR;
		}
		else
		{
			dimOutNewCmd += LOGLAYOUT_NOCODE_OUT;
			dimErrNewCmd += LOGLAYOUT_NOCODE_ERR;
		}
		logOutLen += dimOutNewCmd;
		logErrLen += dimErrNewCmd;

		// log file lenght handling
		if (logOutLen > maxOutLogLenght)
		{
			printf("Log file dimension for the stdout excedeed.\n\n");
			dimension (&fd[0], &maxOutLogLenght);
            logOutLen = dimOutNewCmd;
		}
		if (logErrLen > maxErrLogLenght)
		{
			printf("Log file dimension for the stderr excedeed.\n\n");
            dimension (&fd[1], &maxErrLogLenght);
            logErrLen = dimErrNewCmd;
		}

		printf("%s", buf);  // print the stdout in the shell
		printf("%s\n", buf2);  // print the stderr in the shell

		// we convert the file descriptor into a stream in order to format the output
		FILE * outLog = fdopen(fd[0], "w");
		FILE * errLog = fdopen(fd[1], "w");

		// STDOUT LOG
		//COMMAND
		fprintf(outLog, "COMMAND:\t%s", cmd);
		//DATE
		strftime(date, sizeof(date), "%c", tm);
		fprintf(outLog, "\n\nDATE:\t\t%s", date);
		//COMMAND OUTPUT
		fprintf(outLog, "\n\nOUTPUT:\n\n%s", buf);
		//COMMAND RETURN CODE
		if (codeFlag == 1)
		{
			fprintf(outLog, "\nRETURN CODE:\t%d\n", returnCode);
		}
		fprintf(outLog, SEPARATOR);
		fflush(outLog);

		// STDERR LOG
		//COMMAND
		fprintf(errLog, "COMMAND:\t%s", cmd);
		//DATE
		fprintf(errLog, "\n\nDATE:\t\t%s", date);
		//COMMAND ERROR OUTPUT
		fprintf(errLog, "\n\nERROR OUTPUT:\n\n%s", buf2);
		//COMMAND RETURN CODE
		if (codeFlag == 1)
		{
			fprintf(errLog, "\nRETURN CODE:\t%d\n", returnCode);
		}
		fprintf(errLog, SEPARATOR);
		fflush(errLog);

		// NO, because fclose closes the file descriptor too (consequently seg fault)
		//fclose(outLog);
		//fclose(errLog);

		//TODO: free cmdSplitted
	}
}


void dimension (int * fd, int* logLength)
{
	printf("Type:\n");
	printf("\te\texit\n");
	printf("\to\toverwrite the existing file\n");
	printf("\tc\tcreate a new file");

	char choice;
	do
	{
		printf("\n\t>> ");
		scanf("%c", &choice);

		switch (choice) {
			case 'e':
			case 'E':
				cExit(0);
				break;
			case 'o':
			case 'O':
				{
					// ftruncate truncates the file at specified lenght. In this way we reset it
					int err = ftruncate(*fd, 0);
					if (err < 0)
					{
						printf("Failed in overwriting the file\n");
						exit(0);
					}
				}
				break;
			case 'c':
			case 'C':
				{
                    close(*fd);
					char name[FILENAMELEN];
                    printf("New file name: ");
					scanf("%s", name);
                    int tempLogLenght = -1;
                    do {
                        printf("New log lenght dimension: ");
                        scanf("%d", &tempLogLenght);
                    } while (tempLogLenght < MINLOGLEN); //TODO: if the user
                            //has been born from the ass, this might crash
                    *logLength = tempLogLenght;
					*fd = open(name, O_WRONLY | O_CREAT | O_TRUNC, 0777);
					if (*fd < 0)
					{
						printf("Failed in opening new file\n");
						exit(0);
					}
				}
				break;
			default:
				{
					printf("Choice %c not allowed.\n", choice);
					choice = 'a';
				}
				break;
		}
	} while (choice == 'a');
}

void cExit (int code)
{
	printf("Goodbye\n");
	exit(code);
}

char ** splitArgs (const char * cmd)
{
	// tmp is used because after this function the string changes
	char tmp [strlen(cmd)];
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
	return cleancmd;
}