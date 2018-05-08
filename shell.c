#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#define STDOUT 1
#define CMDSIZE 10
#define MAXBUF 4096
#define SEPARATOR_LEN 50

// extracts filename in args
char * estrai (char *source, int index);
// extracts a substring between given indexes
char * substring (char * src, int first, int last);
// handles the commands
char** parseCommand (char * cmd, int * cmds);
// execute the commands
void run (char * cmd, char * outfile, char * errfile, int * fd);

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
		printf("  -o[=\"NAMEFILE\"], --outfile[=\"NAMEFILE\"]\tNAMEFILE is the log file for the stdout\n");
		printf("  -e[=\"NAMEFILE\"], --errfile[=\"NAMEFILE\"]\tNAMEFILE is the log file for the stderr\n\n");
		printf("Optional parameters:\n");
		printf("  -m[=NUMBER], --maxlen[=NUMBER]\t\tNUMBER is the maximum length of log files");
		printf("\n\t\t\t\t\t\t(in number of characters) (4096 by default)\n");
		printf("  -c,--code\t\t\t\t\talso indicates the return code of the commands\n\n");
		exit(0);
	}

	// parameters
	char *outfile = NULL;  // the name of log outfile
	char *errfile = NULL;  // the name of log errfile
	// length set to -1 to check if the user has specified a new length, otherwise 4096
	int logfileLenght = -1;
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
	}

	if (logfileLenght == -1)  // if the maximum length has not been specified
		logfileLenght = MAXBUF;  // default

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

	while (1) {
		fprintf(stdout, ">> ");
		fflush(stdout);
		read = getline(&line, &len, stdin);
		int cmds=1;  // there's always one command
		char ** cmd = parseCommand(line, &cmds);
		int i;
		run(cmd[0],outfile,errfile, fd);  // TODO: pass other parameters like logfileLenght and code
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
		printf("Goodbye\n");
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

void run (char * cmd, char * outfile, char * errfile, int * fd)
{
	pid_t pid;
	int errorCode;
	pid = fork();

    if (pid < 0)
	{
        printf("Error forking!\n");
        exit(1);
    }
    else if (pid == 0) // child
	{
	//	printf("Command %s, child process\n", cmd);
		int fdOut = open("tmpOut.txt", O_WRONLY | O_CREAT | O_TRUNC, 0777);
		int fdErr = open("tmpErr.txt", O_WRONLY | O_CREAT | O_TRUNC, 0777);

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

		int fdOut = open("tmpOut.txt", O_RDONLY, 0777);
		int fdErr = open("tmpErr.txt", O_RDONLY, 0777);

        char buf[MAXBUF];
        int dim = read(fdOut,buf,MAXBUF);  // read the output written from the child in tmpOut.txt
        printf("%s\n", buf);  // print the output in the shell

        write(fd[0], "COMMAND:\t", strlen("COMMAND:\t"));
		write(fd[0], cmd, strlen(cmd));
		write(fd[0], "\n\nOUTPUT:\n", strlen("\n\nOUTPUT:\n"));
		write(fd[0], buf, dim);  // write in the out log file
		dim = sprintf(buf, "\nRETURN CODE:\t%d", errorCode);
		write(fd[0], buf, dim);
		write(fd[0], "\n------------------------------------------------\n", SEPARATOR_LEN);

		dim = read(fdErr,buf,MAXBUF);  // read the stderr in the tmpErr.txt file
		write(fd[1], "COMMAND:\t", strlen("COMMAND:\t"));
		write(fd[1], cmd, strlen(cmd));
		write(fd[1], "\n\nERROR OUTPUT:\n", strlen("\n\nERROR OUTPUT:\n"));
		write(fd[1], buf, dim);  // write in the out log file
		dim = sprintf(buf, "\nRETURN CODE:\t%d", errorCode);
		write(fd[1], buf, dim);
		write(fd[1], "\n------------------------------------------------\n", SEPARATOR_LEN);

		// close file descriptors
		close(fdOut);
		close(fdErr);
    }
}
