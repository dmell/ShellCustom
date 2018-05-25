#include "commons.h"
#include "shellutil.h"

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
				char * logfileLenghtString;
				if (shortFlag)
				{
					logfileLenghtString = substring(argv[i], 3, strlen(argv[i])-1);
					logfileLenght = atoi(logfileLenghtString);
				}
				else
				{
					logfileLenghtString = substring(argv[i], 9, strlen(argv[i])-1);
					logfileLenght = atoi(logfileLenghtString);
				}
				free(logfileLenghtString);
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
				char * bufLenghtString;
				if (shortFlag)
				{
					bufLenghtString = substring(argv[i], 3, strlen(argv[i])-1);
					bufLenght = atoi(bufLenghtString);
				}
				else
				{
					bufLenghtString = substring(argv[i], 7, strlen(argv[i])-1);
					bufLenght = atoi(bufLenghtString);
				}
				free(bufLenghtString);
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
			endCommand = i-1;  // point where the commands end
			if ((*line)[i+1] == '>')  // check if there is > or >>
			{
				i++;
				*doubleChar = 1;
			}
			while ((*line)[i+1] == ' ')  // delete empty space before the name of the file
			{
				i++;
			}
			name = substring(*line, i+1, strlen(*line)-1);
			newline = substring(*line, 0, endCommand);
			bzero(*line, strlen(*line));
			strcpy(*line, newline);
			free(newline);
			*out = 1;

			int a=strlen(name)-1;
			while (name[a] == ' ')  // remove spaces at the end of the name
			{
				name[a]='\0';
				a--;
			}

			return name;
		}
		else if ((*line)[i] == '<')
		{
			endCommand = i-1;
			if ((*line)[i+1] == '<')  // check if there is < or <<
			{
				i++;
				*doubleChar = 1;
			}
			while ((*line)[i+1] == ' ')  // delete empty space before the name of the file
			{
				i++;
			}
			name = substring(*line, i+1, strlen(*line)-1);
			newline = substring(*line, 0, endCommand);
			bzero(*line, strlen(*line));
			strcpy(*line, newline);
			free(newline);
			*out = 0;

			int a=strlen(name)-1;
			while (name[a] == ' ')  // remove spaces at the end of the name
			{
				name[a]='\0';
				a--;
			}

			return name;
		}
	}
	return name;  // return NULL, not setted
}

char * dimension (FILE * fd, int* logLength)
{
	char * name = malloc(MAXLENGHT_FILENAME); // used only if the user chooses to create a new file
	bzero(name, MAXLENGHT_FILENAME);
	int littlePipe[2];
	pipe(littlePipe);

	pid_t pid = fork();
	if (pid == 0) // child
	{
		dup2(stdin_restore, 0);
		dup2(stdout_restore, 1);
		printf("Type:\n");
		printf("\te\texit\n");
		printf("\to\toverwrite the existing file\n");
		printf("\tc\tcreate a new file");

		// for getline
		char *line = NULL;
		size_t len = 0;
		ssize_t read = 0;

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
					exit(69);
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
	                    } while (tempLogLenght < MINLOGLEN);
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
		close(littlePipe[READ]);
		write(littlePipe[WRITE], name, strlen(name));
		close(littlePipe[WRITE]);
		exit(0);
	}
	int returnCode;
	wait(&returnCode);
	if (WEXITSTATUS(returnCode) == 69)
		cExit(0);
	close(littlePipe[WRITE]);
	read(littlePipe[READ], name, MAXLENGHT_FILENAME);
	close(littlePipe[READ]);
	if (name[0] == '\0')
		return NULL;
	printf("\nNew file name is %s\n\n", name);
	return name;
}

void cExit (int code)
{
	printf("Goodbye\n");
	exit(code);
}
