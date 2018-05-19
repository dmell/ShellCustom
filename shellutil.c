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
	printf("\n\t\t\t\t\t\t(in number of characters, 1000000 char is about 1Mb) (%d by default)\n\n", DEFAULTLOGLEN);
	printf("  -c,--code\t\t\t\t\talso indicates the return code of the commands\n\n");
	printf("  -s[=NUMBER], --size[=NUMBER]\t\t\tNUMBER is the maximum length of command response");
	printf("\n\t\t\t\t\t\t(in number of characters, 1000000 char is about 1Mb) (%d by default)\n", MAXBUF);
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

char * substring (char * src, int first, int last)
{
	//printf("%s\n", src);
	char * res = malloc(last - first + 2);
	int i;
	int j = 0;
	for (int i = first; i <= last; i++)
	{
		res[j] = src[i];
		j++;
	}
	res[j] = '\0';
	//printf("%s\n", res);
	return res;
}

void run (char ** cmd, const int cmds, FILE ** fd, int codeFlag, int bufLenght, int logfileLenght)
{
	int in_restore = dup(0);  // we will use in_restore to restore stdin after dup2
    for(int i = 0; i < cmds; i++)
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
    	char ** cmdSplitted = splitArgs(cmd[i]);
    	printf("%s %s %s\n", cmdSplitted[0], cmdSplitted[1], cmdSplitted[2]);

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

    		//printf("Helo\n"); // magic function that makes everything work
    		execvp(cmdSplitted[0], cmdSplitted);
    		// the following lines will be executed only if the exec has failed
    		int commandError = errno;
    		fprintf(stderr,"%s: command not found\n",cmdSplitted[0]);
    		//write(fdIPC_err[WRITE], "Command not found\n", strlen("Command not found\n"));
    		exit(commandError);
    	}
    	else // parent
    	{
    		//printf("Back to the parent\n");

    		int returnCode;
    		wait(&returnCode);
    		returnCode = WEXITSTATUS(returnCode);
    		close(fdIPC_err[WRITE]);

    		//INITIALIZING BUFFERS
    		char buf[MAXBUF]; // stdout buff
    		char buf2[MAXBUF]; // stderr buff
    		char date[DATESIZE];
    		bzero(buf, MAXBUF); // clean the stdout buffer
    		bzero(buf2, MAXBUF); // clean the stderr buffer
    		bzero(date, DATESIZE); // clean the date buffer

    		int dim = read(fdIPC_out[READ], buf, MAXBUF);  // read the output written from the child in pipe
    		int dim2 = read(fdIPC_err[READ], buf2, MAXBUF);

			if (i != cmds-1)  // if this is not the last command we send the output to the next command
			{
				write(fdIPC_out[WRITE], buf, dim);
				dup2(fdIPC_out[READ],0);
			}
			else
			{
				dup2(in_restore,0);
			}
			close(fdIPC_out[WRITE]);

    		// close file descriptors, we don't need to communicate with the child anymore
    		close(fdIPC_out[READ]);
    		close(fdIPC_err[READ]);

    		// N.B.: we read MAXBUF character and then we will check if it is less then bufLenght
    		if (dim > bufLenght)
    		{
    			printf("The output of the command is too long.\n\n");
    			return;
    		}
    		int dimOutNewCmd = strlen(cmd[i]) + dim; // we want to print dim characters as the output of the cmd
    		int dimErrNewCmd = strlen(cmd[i]) + dim2;
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
    			char * newfilename = dimension (fd[0], &maxOutLogLenght);
    			if (newfilename != NULL)
    			{
    				fclose(fd[0]);
    				fd[0] = fopen(newfilename, "w");
    				if (fd[0] == NULL)
    				{
    					perror("Opening new file");
    				}
    			}
                logOutLen = dimOutNewCmd;
    		}
    		if (logErrLen > maxErrLogLenght)
    		{
    			printf("Log file dimension for the stderr excedeed.\n\n");
                dimension (fd[1], &maxErrLogLenght);
                logErrLen = dimErrNewCmd;
    		}

            if(i == cmds-1)
            {
    	        printf("%s", buf);  // print the stdout in the shell
    	        printf("%s\n", buf2);  // print the stderr in the shell
            }

    		// STDOUT LOG
    		//COMMAND
    		fprintf(fd[0], "COMMAND:\t%s", cmd[i]);
    		//DATE
    		strftime(date, sizeof(date), "%c", tm);
    		fprintf(fd[0], "\n\nDATE:\t\t%s", date);
    		//COMMAND OUTPUT
    		fprintf(fd[0], "\n\nOUTPUT:\n\n%s", buf);
    		//COMMAND RETURN CODE
    		if (codeFlag == 1)
    		{
    			fprintf(fd[0], "\nRETURN CODE:\t%d\n", returnCode);
    		}
    		fprintf(fd[0], SEPARATOR);
    		fflush(fd[0]);

    		// STDERR LOG
    		//COMMAND
    		fprintf(fd[1], "COMMAND:\t%s", cmd[i]);
    		//DATE
    		fprintf(fd[1], "\n\nDATE:\t\t%s", date);
    		//COMMAND ERROR OUTPUT
    		fprintf(fd[1], "\n\nERROR OUTPUT:\n\n%s", buf2);
    		//COMMAND RETURN CODE
    		if (codeFlag == 1)
    		{
    			fprintf(fd[1], "\nRETURN CODE:\t%d\n", returnCode);
    		}
    		fprintf(fd[1], SEPARATOR);
    		fflush(fd[1]);
        }

        for (int i = 0; i < CMDSIZE/2; i++)
    	{
    		free(cmdSplitted[i]);
    	}
    	free(cmdSplitted);
	}
	return;
}


char * dimension (FILE * fd, int* logLength)
{
	printf("Type:\n");
	printf("\te\texit\n");
	printf("\to\toverwrite the existing file\n");
	printf("\tc\tcreate a new file");

	// for getline
	char *line = NULL;
	size_t len = 0;
	ssize_t read = 0;

	char *name = NULL; // used only if the user chooses to create a new file

	char choice;
	do
	{
		printf("\n\t>> ");
		read = getline(&line, &len, stdin);
		if (read != 2)
		{
			line[0] = 'a';  // 'a' represents invalid input
		}

		switch (line[0]) {
			case 'e':
			case 'E':
				cExit(0);
				break;
			case 'o':
			case 'O':
				{
					// ftruncate truncates the file at specified lenght. In this way we reset it
					// fileno return the file descriptor of a FILE*
					int err = ftruncate(fileno(fd), 0);
					if (err < 0)
					{
						printf("Failed in overwriting the file\n");
						exit(1);
					}
				}
				break;
			case 'c':
			case 'C':
				{
					char *inputLen = NULL;
                    printf("New file name: ");
					read = getline(&name, &len, stdin);
					name = substring(name, 0, read-2); // delete last character \n
                    int tempLogLenght = -1;
                    do {
                        printf("New log lenght dimension: ");
						read = getline(&inputLen, &len, stdin);
						tempLogLenght = atoi(inputLen);

						if (tempLogLenght < MINLOGLEN)
						{
							printf("Log lenght dimension is too short. Minimum allowed size: %d\n", MINLOGLEN);
						}
                    } while (tempLogLenght < MINLOGLEN); //TODO: if the user
                            //has been born from the ass, this might crash
                    *logLength = tempLogLenght;
				}
				break;
			default:
				{
					printf("Choice %c not allowed.\n", choice);
					choice = 'a';
				}
				break;
		}
	} while (line[0] == 'a');

	return name;
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
