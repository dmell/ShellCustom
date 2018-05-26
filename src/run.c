#include "commons.h"
#include "shellutil.h"
#include "parsers.h"
#include "run.h"

int run (char ** cmd, const int cmds, FILE ** f)
{
	int returnCode;
	int i;
    for(i = 0; i < cmds; i++)
    {
		// redirezionamento
		int out = 0; // used as boolean to check if there's a < or a > character
		int doubleChar = 0; // used as boolean to check if there's >/< or >>/<<
		// find < or >, return the filename and delete it from the command
		char * redirectFileName = redirect(&cmd[i], &out, &doubleChar);
		//printf("Parsing redirezionamento: %s\n", commands[0]);
		int redirectFdOut, redirectFdIn;
		if (redirectFileName != NULL)
		{
			if (out == 1)
			{
				if (doubleChar == 0)  // a single >
				{
					redirectFdOut = open(redirectFileName, O_WRONLY | O_TRUNC | O_CREAT, 0777);
				}
				else // a double >>
				{
					redirectFdOut = open(redirectFileName, O_WRONLY | O_APPEND | O_CREAT, 0777);
				}
				dup2(redirectFdOut,STDOUT_FILENO);
			}
			else
			{
				redirectFdIn = open(redirectFileName, O_RDONLY, 0777);
				if (redirectFdIn == -1)  // TODO support for << command
				{
					fprintf(stderr, "shell: the file %s does not exist!\n\n", redirectFileName);
					dup2(stdin_restore,STDIN_FILENO);  // restore stdin in case it has already been redirected for a piping
					return 0; // an error has occurred, run return false
				}
				dup2(redirectFdIn,STDIN_FILENO);
			}
		}


    	//pid_t pid;
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
    		dup2(fdIPC_out[WRITE], STDOUT_FILENO);
    		dup2(fdIPC_err[WRITE], STDERR_FILENO);

    		execvp(cmdSplitted[0], cmdSplitted);
    		//int commandError = errno;
    		fprintf(stderr,"%s: command not found\n",cmdSplitted[0]);
    		exit(255);
    	}
    	else // parent
    	{
    		signal(SIGSTOP, handler);
    		wait(&returnCode);
    		returnCode = WEXITSTATUS(returnCode);
			close(fdIPC_out[WRITE]);
    		close(fdIPC_err[WRITE]);
    		signal(SIGINT, SIG_DFL);

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
				dup2(fdIPC_out[READ],STDIN_FILENO);
				close(fdIPC_out[READ]);
				write(fdIPC_out[WRITE], buf, dim);
				close(fdIPC_out[WRITE]);

			}
			else
			{
				dup2(stdin_restore,STDIN_FILENO);
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

    		int j;
    		if(returnCode == 255) // if the command does not exists
    		{
    			strcat(logOutBuf, "SYSTEM: sintax error for not valid command ");
	    		for (j = 0; j < cmds; j++)
	    		{
	    			strcat(logOutBuf, cmd[j]);
	    			if (j != cmds-1)
	    				strcat(logOutBuf, " | ");
	    		}
    		}
    		else
    		{
    			// STDOUT LOG
	    		strcat(logOutBuf, "COMMAND:\t");
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
	    			char * tella = malloc(3);
	    			sprintf(tella, "%d", returnCode);
	    			strcat(logOutBuf, tella);
	    			free(tella);
	    		}
	    	}
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
    			char * tella = malloc(3);
    			sprintf(tella, "%d", returnCode);
    			strcat(logErrBuf, tella);
    			free(tella);
    		}

    		strcat(logOutBuf, SEPARATOR);
    		strcat(logErrBuf, SEPARATOR);

    		logOutLen += strlen(logOutBuf);
    		logErrLen += strlen(logErrBuf);

    		// log file lenght handling
			if (logOutLen > maxOutLogLenght)
    		{
    			printf("Log file dimension for the stdout excedeed.\n\n");
    			char * newfilename = dimension (f[0], &maxOutLogLenght);
    			if (newfilename != NULL)
    			{
    				fclose(f[0]);
    				f[0] = fopen(newfilename, "w");
    				if (f[0] == NULL)
    				{
    					perror("Opening new file");
    				}
    				free(newfilename);
    			}
                logOutLen = strlen(logOutBuf);
    		}
    		if (logErrLen > maxErrLogLenght)
    		{
    			printf("Log file dimension for the stderr excedeed.\n\n");
                char * newfilename = dimension (f[1], &maxErrLogLenght);
                if (newfilename != NULL)
    			{
    				fclose(f[1]);
    				f[1] = fopen(newfilename, "w");
    				if (f[1] == NULL)
    				{
    					perror("Opening new file");
    				}
    				free(newfilename);
    			}
                logErrLen = strlen(logErrBuf);
    		}

    		fprintf(f[0], "%s", logOutBuf);
    		fflush(f[0]);
    		fprintf(f[1], "%s", logErrBuf);
    		fflush(f[1]);

            if (killed != 0) // normal process flow
            {
            	if(i == cmds-1 )
            	{
    	        	printf("%s", buf);  // print the stdout in the shell
            	}
            	printf("%s\n", buf2);  // print the stderr in the shell
            }
            killed = 1;

			if (redirectFileName != NULL)  // restore the normal stdin and stdout if redirected
			{
				if (out == 1)
				{
					dup2(stdout_restore,STDOUT_FILENO);
					close(redirectFdOut);
				}
				else
				{
					dup2(stdin_restore,STDIN_FILENO);
					close(redirectFdIn);
				}
				free(redirectFileName);
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

void handler (int sig)
{
	fprintf(stderr, " Interactive commands not supported.\n");
	killed = kill(pid, SIGKILL);
}
