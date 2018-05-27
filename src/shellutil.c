/*

library of various functions used in the program

*/



#include "commons.h"
#include "shellutil.h"

// extract the values of the comman line parameters given by the user when program is run
void checkParameters(int argc, char ** argv)
{
	int shortFlag; // we will use this to handle the double option to gives args
	int i;
	for (i = 1; i < argc; i++)
	{
		// if man has been requested
		if (strcmp("--help",argv[i]) == 0)
		{
			showManual();  // NB showManual will exit
		}

		if (strncmp(argv[i], "-o=", 3) == 0 || strncmp(argv[i], "--outfile=", 10) == 0)  // outfile
		{
			shortFlag = strncmp(argv[i], "--outfile=", 10);
			if (outfile == NULL)  // not set yet
			{
				if (shortFlag)
				{
					outfile = substring(argv[i], 3, strlen(argv[i])-1);  // extract out file name
				}
				else
				{
					outfile = substring(argv[i], 10, strlen(argv[i])-1);  // extract out file name
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
					errfile = substring(argv[i], 3, strlen(argv[i])-1);  // extract err file name
				}
				else
				{
					errfile = substring(argv[i], 10, strlen(argv[i])-1);  // extract err file name
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
					// extract logfileLenght
					logfileLenghtString = substring(argv[i], 3, strlen(argv[i])-1);
					logfileLenght = atoi(logfileLenghtString);  // set logfileLenght
				}
				else
				{
					// extract logfileLenght
					logfileLenghtString = substring(argv[i], 9, strlen(argv[i])-1);
					logfileLenght = atoi(logfileLenghtString);  // set logfileLenght
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
		// find code flag that allow to print return code in the logs
		else if (strncmp(argv[i], "-c", 2) == 0 || strncmp(argv[i], "--code", 6) == 0)
		{
			code = 1; // the flag is set to include return code of the commands
		}
		else if (strncmp(argv[i], "-s=", 3) == 0 || strncmp(argv[i], "--size=", 7) == 0)  // buffer lenght
		{
			shortFlag = strncmp(argv[i], "--size=", 7);
			if (bufLenght == -1)  // not set yet
			{
				char * bufLenghtString;
				if (shortFlag)
				{
					// extract bufLenght
					bufLenghtString = substring(argv[i], 3, strlen(argv[i])-1);
					bufLenght = atoi(bufLenghtString);  // set bufLenght
				}
				else
				{
					// extract bufLenght
					bufLenghtString = substring(argv[i], 7, strlen(argv[i])-1);
					bufLenght = atoi(bufLenghtString);  // set bufLenght
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

	if (logfileLenght == -1)  // if the maximum length for the file has not been specified
		logfileLenght = DEFAULTLOGLEN;  // default

	if (bufLenght == -1)  // if the maximum length for the output has not been specified
		bufLenght = DEFAULTBUFLEN;  // default


	// logfileLenght has a minimum value
	// output can't be longer than logfileLenght
	if (logfileLenght < MINLOGLEN || logfileLenght < bufLenght)
	{
		printf("shell: error in buffer or file size.\n");
		printf("Try './shell --help' for more information.\n");
		exit(1);
	}

	if (bufLenght < MINBUFLEN)
	{
		printf("shell: error in buffer or file size.\n");
		printf("Try './shell --help' for more information.\n");
		exit(1);	
	}

	// we check that the user has specified the log files (mandatory)
	if (outfile == NULL || errfile == NULL)
	{
		printf("shell: missing mandatory parameter.\n");
		printf("Try './shell --help' for more information.\n");
		exit(1);
	}
}

// print user manual and exit
void showManual()
{
	printf("Usage: ./shell [PARAMETERS]\n");
	printf("Run the Custom Shell\n\n");
	printf("Mandatory parameters:\n");
	printf("  -o[=\"NAMEFILE\"], --outfile[=\"NAMEFILE\"]\tNAMEFILE is the log file for the stdout.\n\n");
	printf("  -e[=\"NAMEFILE\"], --errfile[=\"NAMEFILE\"]\tNAMEFILE is the log file for the stderr.\n\n");
	printf("Optional parameters:\n");
	printf("  -c,--code\t\t\t\t\talso indicates the return code of the commands.\n\n");
	printf("  -m[=NUMBER], --maxlen[=NUMBER]\t\tNUMBER is the maximum length of log files");
	printf("\n\t\t\t\t\t\t(in number of characters, 1000000 char is about 1Mb) (%d by default).\n", DEFAULTLOGLEN);
	printf("\t\t\t\t\t\tmaxlen can not be shorter than %d.\n\n", MINLOGLEN);
	printf("  -s[=NUMBER], --size[=NUMBER]\t\t\tNUMBER is the maximum length of command response");
	printf("\n\t\t\t\t\t\t(in number of characters, 1000000 char is about 1Mb) (%d by default).\n", DEFAULTBUFLEN);
	printf("\t\t\t\t\t\tsize can not be shorter than %d.\n\n", MINBUFLEN);
	exit(0);
}


// check if there's a redirect character < << >
// in case of redirection, the function delete the name of the file from the
// command and returns the name of the file.
// flag out passed by reference distinguish an in or out redirection
// flag doubleChar passed by reference distinguish append command <<
char * redirect(char ** line, int * out, int * doubleChar)
{
	char * name = NULL;  // return value
	char * newline = NULL;  // tmp support string
	int i;
	int endCommand;
	for (i=0; i < strlen(*line); i++)
	{
		if ((*line)[i] == '>')  // find a >
		{
			endCommand = i-1;  // point where the commands end (for the extraction of the command)
			if ((*line)[i+1] == '>')  // check if there is >>
			{
				i++;
				*doubleChar = 1;
			}
			while ((*line)[i+1] == ' ')  // delete empty space before the name of the file
			{
				i++;
			}
			name = substring(*line, i+1, strlen(*line)-1);  // extract the file name
			// delete the name of the file from the command
			newline = substring(*line, 0, endCommand);
			bzero(*line, strlen(*line));
			strcpy(*line, newline);
			free(newline);
			*out = 1;  // set out flag, there's an output redirection

			int a=strlen(name)-1;
			while (name[a] == ' ')  // remove spaces at the end of the name
			{
				name[a]='\0';
				a--;
			}

			return name;
		}
		else if ((*line)[i] == '<')  // find a <
		{
			endCommand = i-1;  // point where the commands end (for the extraction of the command)
			if ((*line)[i+1] == '<')  // check if there is <<
			{
				i++;
				*doubleChar = 1;
			}
			while ((*line)[i+1] == ' ')  // delete empty space before the name of the file
			{
				i++;
			}
			name = substring(*line, i+1, strlen(*line)-1);  // extract the name
			// delete the name of the file from the command
			newline = substring(*line, 0, endCommand);
			bzero(*line, strlen(*line));
			strcpy(*line, newline);
			free(newline);
			*out = 0;  // set the out flag, there's an input redirection

			int a=strlen(name)-1;
			while (name[a] == ' ')  // remove spaces at the end of the name
			{
				name[a]='\0';
				a--;
			}

			return name;
		}
	}
	return name;  // return NULL, if not setted
}


// handling of a log file dimension limit
// the function let the user choose between exit from the program,
// overwrite the current log file (with ftruncate on the file descriptor)
// or create a new log file. In this last case the function return the new
// file name and the new dimension to run function, that changes the settings
char * dimension (FILE * fd, int* logLength)
{
	char * name = malloc(MAXLENGHT_FILENAME); // used only if the user chooses to create a new file
	bzero(name, MAXLENGHT_FILENAME);

	// this pipe allow to get a new file name from the child of the fork
	int littlePipe[2];
	pipe(littlePipe);

	// a new process allow to avoid problems with stdin/stdout duplicated in other points of the program
	pid_t pid = fork();
	if (pid == 0) // child
	{
		dup2(stdin_restore, 0);  // restore normale stdin or stdout for user interaction
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
			read = getline(&line, &len, stdin);  // read user choice
			if (read != 2)
			{
				line[0] = 'a';  // 'a' represents invalid input
			}

			switch (line[0]) {
				case 'e':
				case 'E':
					exit(69);  // user chooses to exit
					break;
				case 'o':
				case 'O':
					{
						// user choose to overwrite the current file
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
						// user choose to create a new file
						char *inputLen = NULL;
	                    printf("New file name: ");
						read = getline(&name, &len, stdin);
						name = substring(name, 0, read-2); // delete last character \n
	                    int tempLogLenght = -1;
	                    do {
	                        printf("New log lenght dimension: ");
							read = getline(&inputLen, &len, stdin);
							tempLogLenght = atoi(inputLen);

							if (tempLogLenght < MINLOGLEN)  // check minimum file lenght
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

		// write the new file name in the pipe
		close(littlePipe[READ]);
		write(littlePipe[WRITE], name, strlen(name));
		close(littlePipe[WRITE]);
		exit(0);
	}

	int returnCode;
	wait(&returnCode);
	if (WEXITSTATUS(returnCode) == 69)  // user chose to exit
	{
		printf("Goodbye\n");
		exit(0);
	}

	close(littlePipe[WRITE]);  // we need only to read the file name
	read(littlePipe[READ], name, MAXLENGHT_FILENAME);
	close(littlePipe[READ]);

	if (name[0] == '\0')  // user chose to overwrite the current file
		return NULL;

	printf("\nNew file name is %s\n\n", name);
	return name;  // user choose a new file for the log
}
