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
#define STDOUT 1
#define CMDSIZE 10
#define DEFAULTLOGLEN 4096
#define MINLOGLEN 512
#define MAXBUF 512
#define SEPARATOR "\n------------------------------------------------\n\n"
#define SEPARATOR_LEN 51
#define DATESIZE 32
#define LOGLAYOUT_CODE 120
#define LOGLAYOUT_NOCODE 105

// extracts filename in args
char * estrai (char *source, int index);
// extracts a substring between given indexes
char * substring (char * src, int first, int last);
// handles the commands
char** parseCommand (char * cmd, int * cmds);
// execute the commands
void run (char * cmd, char * outfile, char * errfile, int * fd, int codeFlag, int bufLenght, int logfileLenght);

int logOutLen = 0;
int logErrLen = 0;

int main(int argc, char **argv)
{
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
		exit(1);
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

char** parseCommand (char * cmd, int * cmds)
{
	if (strncmp(cmd, "exit", 4) == 0)
	{
		printf("Goodbye\nP.S.: Damiano ricordati di fare make clean prima di modificare il sorgente\n");
		exit(0);
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
	int errorCode;
	// date and time of the execution
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
	pid = fork();

    if (pid < 0)
	{
        printf("Error forking!\n");
        exit(1);
    }
    else if (pid == 0) // child
	{
	//	printf("Command %s, child process\n", cmd);
		/* TODO: decide if it is better to open the temporary logs twice (once in write only
		and once in read only) or opening them only once.
		*/
		int fdOut = open("../src/tmp/tmpOut.txt", O_WRONLY | O_CREAT | O_TRUNC, 0777);
		int fdErr = open("../src/tmp/tmpErr.txt", O_WRONLY | O_CREAT | O_TRUNC, 0777);

		// dup2 is counterintuitive: the output of the second fd goes to the first
		dup2(fdOut,1);  // write the stdout of the command in tmpOut.txt
		dup2(fdErr,2);  // write the stderr of the command in tmpErr.txt

        errorCode = system(cmd);
		close(fdOut);
		close(fdErr);
		exit(0);
    }
    else // parent
	{
        wait(NULL);  // wait for the child
        //printf("Back to the parent\n");

		int fdOut = open("../src/tmp/tmpOut.txt", O_RDONLY, 0777);
		int fdErr = open("../src/tmp/tmpErr.txt", O_RDONLY, 0777);

        //INITIALIZING BUFFERS
        char buf[MAXBUF]; // stdout buff
        char buf2[MAXBUF]; // stderr buff
        char date[DATESIZE];
        bzero(buf, MAXBUF); // clean the stdout buffer
        bzero(buf2, MAXBUF); // clean the stderr buffer
        bzero(date, DATESIZE); //clean the date buffer

        int dim = read(fdOut,buf,MAXBUF);  // read the output written from the child in tmpOut.txt
        int dim2 = read(fdErr, buf2, MAXBUF); 
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
        	dimOutNewCmd += LOGLAYOUT_CODE;
        	dimErrNewCmd += LOGLAYOUT_CODE;
        }
        else
        {
        	dimOutNewCmd += LOGLAYOUT_NOCODE;
        	dimErrNewCmd += LOGLAYOUT_NOCODE;
        }
        logOutLen += dimOutNewCmd;
        logErrLen += dimErrNewCmd;

        if (logOutLen > logfileLenght || logErrLen > logfileLenght)
        {
        	// in this case our idea is to print a sort of menù, to let the user decide between
        	// starting a new file, overwriting the existing one, or simply exiting.
        	//showAlert(); // TODO
        	printf("Log file dimension excedeed.\n\n");
        }

        printf("%s", buf);  // print the output in the shell

        //COMMAND
        write(fd[0], "COMMAND:\t", strlen("COMMAND:\t"));
		write(fd[0], cmd, strlen(cmd));
		//DATE
		strftime(date, sizeof(date), "%c", tm);
		write(fd[0], "\n\nDATE:\t\t", strlen("\n\nDATE:\t\t"));
		write(fd[0], date, strlen(date));
        //COMMAND OUTPUT
		write(fd[0], "\n\nOUTPUT:\n\n", strlen("\n\nOUTPUT:\n\n"));
		write(fd[0], buf, dim);  // write in the out log file
        //COMMAND RETURN CODE
		if (codeFlag == 1)
		{
			bzero(buf, MAXBUF); // better clean the buffer every time we need it 
			dim = sprintf(buf, "\nRETURN CODE:\t%d\n", errorCode);
			write(fd[0], buf, dim);
		}
		write(fd[0], SEPARATOR, SEPARATOR_LEN);

        printf("%s\n", buf2);  // print the output in the shell
        //COMMAND
		write(fd[1], "COMMAND:\t", strlen("COMMAND:\t"));
		write(fd[1], cmd, strlen(cmd));
        //DATE
		write(fd[1], "\n\nDATE:\t\t", strlen("\n\nDATE:\t\t"));
		write(fd[1], date, strlen(date));
        //COMMAND ERROR OUTPUT
		write(fd[1], "\n\nERROR OUTPUT:\n\n", strlen("\n\nERROR OUTPUT:\n\n"));
		write(fd[1], buf2, dim);  // write in the out log file
        //COMMAND RETURN CODE
		if (codeFlag == 1)
		{
			// TODO: introduce a new string named returnCode to avoid repeating this instruction twice
			bzero(buf2, MAXBUF);
			dim = sprintf(buf2, "\nRETURN CODE:\t%d\n", errorCode);
			write(fd[1], buf2, dim);
		}
		write(fd[1], SEPARATOR, SEPARATOR_LEN);

		// close file descriptors
		close(fdOut);
		close(fdErr);

    }
}
