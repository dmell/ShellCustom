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
	int errorCode;
	// date and time of the execution
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
	pid = fork();

    if (pid < 0)
	{
        printf("Error forking!\n");
        cExit(1);
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

        errorCode = system(cmd);  // TODO: NB errorcode e' il valore di ritorno della system, non di cmd
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
        if (logOutLen > logfileLenght)
        {
        	// in this case our idea is to print a sort of menù, to let the user decide between
        	// starting a new file, overwriting the existing one, or simply exiting.
        	//showAlert(); // TODO
        	printf("Log file dimension for the stdout excedeed.\n\n");
			//dimension ();
        }
		if (logErrLen > logfileLenght)
		{
			printf("Log file dimension for the stderr excedeed.\n\n");

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
		write(fd[1], buf2, dim2);  // write in the out log file
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


void dimension ()
{
	printf("Type:\n");
	printf("\te\texit\n");
	printf("\to\toverwrite the existing file\n");
	printf("\tc\tcreate a new file\n");

	char choice;
	scanf("%c\n", &choice);

	switch (choice) {
		case 'e':
        case 'E':
		case 'o':
        case 'O':
		case 'c':
        case 'C':
	}
}

void cExit (int code)
{
    printf("Goodbye\n");
    exit(code);
}
