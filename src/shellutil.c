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

void checkParameters(int argc, char ** argv)
{
	int shortFlag; // we will use this to handle the double option to gives args
	int i;
	for (i = 1; i < argc; i++)
	{
		// if man has been requested
		if (strcmp("--help",argv[i]) == 0)
		{
			showManual();
		}

		if (strncmp(argv[i], "-o=", 3) == 0 || strncmp(argv[i], "--outfile=", 10) == 0)  // outfile
		{
			shortFlag = strncmp(argv[i], "--outfile=", 10);
			if (outfile == NULL)  // not set yet
			{
				if (shortFlag)
				{
					outfile = substring(argv[i], 3, strlen(argv[i])-1);
				}
				else
				{
					outfile = substring(argv[i], 10, strlen(argv[i])-1);
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
					errfile = substring(argv[i], 3, strlen(argv[i])-1);
				}
				else
				{
					errfile = substring(argv[i], 10, strlen(argv[i])-1);
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
					logfileLenght = atoi(substring(argv[i], 3, strlen(argv[i])-1));
				}
				else
				{
					logfileLenght = atoi(substring(argv[i], 9, strlen(argv[i])-1));
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
					bufLenght = atoi(substring(argv[i], 3, strlen(argv[i])-1));
				}
				else
				{
					bufLenght = atoi(substring(argv[i], 7, strlen(argv[i])-1));
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

	// we check that outfile name is different from errfile NAMEFILE
	if (strcmp(outfile,errfile) == 0)
	{
		printf("shell: outfile and errfile parameters cannot be the same.\n");
		printf("Try './shell --help' for more information.\n");
		exit(1);
	}
}

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
	for (i = 0; i < counter+1; i++)
	{
		multipleCommands[i] = (char*)malloc((CMDSIZE+1)*(sizeof(char)));
	}

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


char** parseCommand (char * cmd, int * cmds)
{
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
	if (cmd[i-1] == '\n')
		cleancmd[j] = substring(cmd, beg, i-2);  // insert last command
	else
		cleancmd[j] = substring(cmd, beg, i-1);  // this case comes up with ||
	return cleancmd;
}

char * redirect(char ** line, int * out, int * doubleChar)
{
	char * name = NULL;
	char * newline = NULL;
	int i;
	int endCommand;
	for (i=0; i < strlen(*line); i++)
	{
		if ((*line)[i] == '>')
		{
			endCommand = i;  // point where the commands end
			if ((*line)[i+1] == '>')  // check if there is > or >>
			{
				i++;
				*doubleChar = 1;
			}
			while ((*line)[i+1] == ' ')  // delete empty space before the name of the file
			{
				i++;
			}
			name = substring(*line, i+1, strlen(*line)-2);
			newline = substring(*line, 0, endCommand);
			bzero(*line, strlen(*line));
			strcpy(*line, newline);
			free(newline);
			*out = 1;
			return name;
		}
		else if ((*line)[i] == '<')
		{
			endCommand = i;
			if ((*line)[i+1] == '<')  // check if there is < or <<
			{
				i++;
				*doubleChar = 1;
			}
			while ((*line)[i+1] == ' ')  // delete empty space before the name of the file
			{
				i++;
			}
			name = substring(*line, i+1, strlen(*line)-2);
			newline = substring(*line, 0, endCommand);
			bzero(*line, strlen(*line));
			strcpy(*line, newline);
			free(newline);
			*out = 0;
			return name;
		}
	}
	return name;
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

char * substring (char * src, int first, int last)
{
	//printf("%s\n", src);
	char * res = malloc(last - first + 2);
	int i;
	int j = 0;
	for (i = first; i <= last; i++)
	{
		res[j] = src[i];
		j++;
	}
	res[j] = '\0';
	//printf("%s\n", res);
	return res;
}

int run (char ** cmd, const int cmds, FILE ** fd)
{
	int in_restore = dup(0); // we will use in_restore to restore stdin after dup2
	//int out_backup = dup(1); // a backup of stdout, for debugging purpose
	//printf("run arguments: %s %s \n", cmd[0], cmd[1]);

	int returnCode;
	int i;
    for(i = 0; i < cmds; i++)
    {
    	pid_t pid;
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
    	//printf("%s %s %s\n", cmdSplitted[0], cmdSplitted[1], cmdSplitted[2]);

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

    		execvp(cmdSplitted[0], cmdSplitted);
    		int commandError = errno;
    		// TODO: setting an error flag to handle different logging
    		fprintf(stderr,"%s: command not found\n",cmdSplitted[0]);
    		//write(fdIPC_err[WRITE], "Command not found\n", strlen("Command not found\n"));
    		exit(commandError);
    	}
    	else // parent
    	{
    		wait(&returnCode);
    		returnCode = WEXITSTATUS(returnCode);
			close(fdIPC_out[WRITE]);
    		close(fdIPC_err[WRITE]);

    		//INITIALIZING BUFFERS
    		char buf[MAXBUF]; // stdout buff
    		char buf2[MAXBUF]; // stderr buff
    		char date[DATESIZE];
    		bzero(buf, MAXBUF); // clean the stdout buffer
    		bzero(buf2, MAXBUF); // clean the stderr buffer
    		bzero(date, DATESIZE); // clean the date buffer
    		strftime(date, sizeof(date), "%c", tm);

    		int dim = read(fdIPC_out[READ], buf, MAXBUF);  // read the output written from the child in pipe
    		int dim2 = read(fdIPC_err[READ], buf2, MAXBUF);

			if (i != cmds-1)  // if this is not the last command we send the output to the next command
			{
				pipe(fdIPC_out);  // we open he pipe again to write, using it for piping
				dup2(fdIPC_out[READ],0);
				close(fdIPC_out[READ]);
				write(fdIPC_out[WRITE], buf, dim);
				close(fdIPC_out[WRITE]);

			}
			else
			{
				dup2(in_restore,0);
			}

    		// close file descriptors, we don't need to communicate with the child anymore
    		close(fdIPC_out[READ]);
    		close(fdIPC_err[READ]);

    		// N.B.: we read MAXBUF character and then we will check if it is less then bufLenght
    		if (dim > bufLenght)
    		{
    			printf("The output of the command is too long.\n\n");
    			return 0;  // run is false, an error has occurred
    		}

    		char logOutBuf [LOGLAYOUT_DIM];
    		char logErrBuf [LOGLAYOUT_DIM];
    		bzero(logOutBuf, LOGLAYOUT_DIM);
    		bzero(logErrBuf, LOGLAYOUT_DIM);

    		// STDOUT LOG
    		strcat(logOutBuf, "COMMAND:\t");
			int j;
    		for (j = 0; j < cmds; j++)
    		{
    			strcat(logOutBuf, cmd[j]);
    			if (j != cmds-1)
    				strcat(logOutBuf, " | ");
    		}
    		if (cmds > 1)
    		{
    			strcat(logOutBuf, "\nSUBCOMMAND:\t");
    			strcat(logOutBuf, cmd[i]);
    		}
    		strcat(logOutBuf, "\n\nDATE:\t\t");
    		strcat(logOutBuf, date);
    		strcat(logOutBuf, "\n\nOUTPUT:\n\n");
    		strcat(logOutBuf, buf);
    		if (code == 1)
    		{
    			strcat(logOutBuf, "\nRETURN CODE:\t");
    			char * returnCodeString = malloc(3);
    			sprintf(returnCodeString, "%d", returnCode);
    			strcat(logOutBuf, returnCodeString);
    			free(returnCodeString);
    		}
    		strcat(logOutBuf, SEPARATOR);

    		// STDERR LOG
    		strcat(logErrBuf, "COMMAND:\t");
    		for (j = 0; j < cmds; j++)
    		{
    			strcat(logErrBuf, cmd[j]);
    			if (j != cmds-1)
    				strcat(logErrBuf, " | ");
    		}
    		if (cmds > 1)
    		{
    			strcat(logErrBuf, "\nSUBCOMMAND:\t");
    			strcat(logErrBuf, cmd[i]);
    		}
    		strcat(logErrBuf, "\n\nDATE:\t\t");
    		strcat(logErrBuf, date);
    		strcat(logErrBuf, "\n\nERROR OUTPUT:\n\n");
    		strcat(logErrBuf, buf2);
    		if (code == 1)
    		{
    			strcat(logErrBuf, "\nRETURN CODE:\t");
    			char * returnCodeString = malloc(3);
    			sprintf(returnCodeString, "%d", returnCode);
    			strcat(logErrBuf, returnCodeString);
    			free(returnCodeString);
    		}
    		strcat(logErrBuf, SEPARATOR);

    		logOutLen += strlen(logOutBuf);
    		logErrLen += strlen(logErrBuf);

    		// log file lenght handling
			if (cmds > 1 && i != cmds-1)  // if this is  one (not the last) of more command we need to restore the stdin for dimension function
			{
				dup2(in_restore,0);
			}
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
                logOutLen = strlen(logOutBuf);
    		}
    		if (logErrLen > maxErrLogLenght)
    		{
    			printf("Log file dimension for the stderr excedeed.\n\n");
                char * newfilename = dimension (fd[1], &maxErrLogLenght);
                if (newfilename != NULL)
    			{
    				fclose(fd[1]);
    				fd[1] = fopen(newfilename, "w");
    				if (fd[1] == NULL)
    				{
    					perror("Opening new file");
    				}
    			}
                logErrLen = strlen(logErrBuf);
    		}
			if (cmds > 1 && i != cmds-1)  // if this is one (not the last) of more command we need to restore the correct stdin for piping
			{
				pipe(fdIPC_out);  // we open he pipe again to write, using it for piping
				dup2(fdIPC_out[READ],0);
				close(fdIPC_out[READ]);
				write(fdIPC_out[WRITE], buf, dim);
				close(fdIPC_out[WRITE]);
			}


    		fprintf(fd[0], "%s", logOutBuf);
    		fflush(fd[0]);
    		fprintf(fd[1], "%s", logErrBuf);
    		fflush(fd[1]);

            if(i == cmds-1)
            {
    	        printf("%s", buf);  // print the stdout in the shell
    	        printf("%s\n", buf2);  // print the stderr in the shell
            }

        }

		int k;
        for (k = 0; k < CMDSIZE/2; k++)
    	{
    		free(cmdSplitted[k]);
    	}
    	free(cmdSplitted);
	}

	if (returnCode == 0)  // command correctly executed
	{
		return 1;  // run is true
	}
	else  // command error
	{
		return 0;  // run is false
	}
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
