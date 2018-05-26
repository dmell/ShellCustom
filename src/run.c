/*

run function handle the real execution of the commands.
run handle the redirections in case of <, > or >> characters.
It saves the output of the execution in a pipe and then print it in the
shell and in the log files, handling also the pipig character.
It returns an int representing the outcome of the execution.

handler function kill the child process. The pid of the current child process
is stored in a global variable.

*/

#include "commons.h"
#include "shellutil.h"
#include "parsers.h"
#include "run.h"

int run (char ** cmd, const int cmds, FILE ** f)
{
	int returnCode;
	int i;
    for(i = 0; i < cmds; i++)  // for each command divided by pipe character (parsed in main)
    {
		// redirecting handling
		int out = 0; // used as boolean to check if there's a < or a > character
		int doubleChar = 0; // used as boolean to check if there's >/< or >>/<<

		// find < or >, return the filename and delete it from the command
		char * redirectFileName = redirect(&cmd[i], &out, &doubleChar);

		int redirectFdOut, redirectFdIn;
		if (redirectFileName != NULL)  // if there's a redirection
		{
			if (out == 1)  // redirection of the stdout
			{
				if (doubleChar == 0)  // a single >
				{
					redirectFdOut = open(redirectFileName, O_WRONLY | O_TRUNC | O_CREAT, 0777);
				}
				else // a double >>
				{
					redirectFdOut = open(redirectFileName, O_WRONLY | O_APPEND | O_CREAT, 0777);
				}
				dup2(redirectFdOut,STDOUT_FILENO);  // redirect stdout
			}
			else  // redirection of the stdin (a single <)
			{
				redirectFdIn = open(redirectFileName, O_RDONLY, 0777);
				if (redirectFdIn == -1)
				{
					fprintf(stderr, "shell: the file %s does not exist!\n\n", redirectFileName);

					// restore stdin in case it has already been redirected for a piping
					dup2(stdin_restore,STDIN_FILENO);
					return 0; // an error has occurred, run return false
				}
				dup2(redirectFdIn,STDIN_FILENO);  // rerirect stdin
			}
		}

		// variables to compute the lenght of log files after the execution of the
		// command (to check if log file becomes too long).
		// Inizialized at the current lenght of the files
        int maxOutLogLenght = logfileLenght;
        int maxErrLogLenght = logfileLenght;

    	// date and time of the execution
    	time_t t = time(NULL);
    	struct tm *tm = localtime(&t);

    	// the idea is to make the two processes (parent and child)
		// comunicate with pipes (one for stdout and one for stderr), in order
    	// to send the output of the command to the parent and make it print.
    	int fdIPC_out[2];
    	int fdIPC_err[2];
    	pipe(fdIPC_out);
    	pipe(fdIPC_err);

    	// we use splitArgs to create an array of strings
		// with the single command and its parameters
    	char ** cmdSplitted = splitArgs(cmd[i]);

    	pid = fork();

    	if (pid < 0)
    	{
    		printf("Error forking!\n");
    		exit(1);
    	}
    	else if (pid == 0) // child
    	{
    		close(fdIPC_out[READ]); // child doesn't read
    		close(fdIPC_err[READ]);
    		dup2(fdIPC_out[WRITE], STDOUT_FILENO);  // child writes in pipes
    		dup2(fdIPC_err[WRITE], STDERR_FILENO);

    		execvp(cmdSplitted[0], cmdSplitted); // execution of the command

			// printed only if execvp returns (in case of error)
    		fprintf(stderr,"%s: command not found\n",cmdSplitted[0]);
    		exit(255);
    	}
    	else // parent
    	{
    		signal(SIGSTOP, handler);  // handling of ^C, if an execution is not responding

			// catch the return code of the execution
    		wait(&returnCode);
    		returnCode = WEXITSTATUS(returnCode); // extract the return code

			close(fdIPC_out[WRITE]); // parent doesn't write in the pipe
    		close(fdIPC_err[WRITE]);

    		signal(SIGINT, SIG_DFL); // restore default ^C handler

    		//INITIALIZING BUFFERS
    		char buf[MAXBUF]; // stdout buffer
    		char buf2[MAXBUF]; // stderr buffer
    		char date[DATESIZE];  // date buffer
    		bzero(buf, MAXBUF); // clean the stdout buffer
    		bzero(buf2, MAXBUF); // clean the stderr buffer
    		bzero(date, DATESIZE); // clean the date buffer

    		strftime(date, sizeof(date), "%c", tm); // write the date in the buf

			// read the output written from the child in the pipes
    		int dim = read(fdIPC_out[READ], buf, MAXBUF);  // read stdout
    		int dim2 = read(fdIPC_err[READ], buf2, MAXBUF);  // read stderr

			if (i != cmds-1)  // if this is not the last command we send the output to the next command
			{
				pipe(fdIPC_out);  // we open he pipe again to write, using it for piping
				dup2(fdIPC_out[READ],STDIN_FILENO);  // next command will read from pipe
				close(fdIPC_out[READ]);  // this command doesn't read
  				// write the output of this program in the pipe. It will be the input of the next command
				write(fdIPC_out[WRITE], buf, dim);
				close(fdIPC_out[WRITE]);  // this command does't write in the pipe anymore
			}
			else  // this is the last command
			{
				dup2(stdin_restore,STDIN_FILENO);  // restore the default stream for stdin
			}

    		// close file descriptors, we don't need to communicate with the child anymore
    		close(fdIPC_out[READ]);
    		close(fdIPC_err[READ]);

    		// we read MAXBUF character and then we will check if it is less then bufLenght
    		if (dim > bufLenght)
    		{
    			printf("The output of the command is too long.\n\n");
    			return 0;  // run is false, an error has occurred
    		}

			// writing in logOutBuf and logErrBuf all we need to write in the log files
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
    		else  // correct execution of the command
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

			// compute the new lenght for log files before writing outputs
    		logOutLen += strlen(logOutBuf);
    		logErrLen += strlen(logErrBuf);

    		// log file lenght handling
			if (logOutLen > maxOutLogLenght)  // max out file lenght exeeded
    		{
    			printf("Log file dimension for the stdout excedeed.\n\n");

				// dimension allow to choose what to do
    			char * newfilename = dimension (f[0], &maxOutLogLenght);
    			if (newfilename != NULL)  // user decided to create a new log file
    			{
    				fclose(f[0]);
    				f[0] = fopen(newfilename, "w");
    				if (f[0] == NULL)
    				{
    					perror("Opening new file");
    				}
    				free(newfilename);
    			}
                logOutLen = strlen(logOutBuf);  // reset current log file lenght
    		}
    		if (logErrLen > maxErrLogLenght)  // max out file lenght exeeded
    		{
    			printf("Log file dimension for the stderr excedeed.\n\n");

				// dimension allow to choose what to do
                char * newfilename = dimension (f[1], &maxErrLogLenght);
                if (newfilename != NULL)  // user decided to create a new log file
    			{
    				fclose(f[1]);
    				f[1] = fopen(newfilename, "w");
    				if (f[1] == NULL)
    				{
    					perror("Opening new file");
    				}
    				free(newfilename);
    			}
                logErrLen = strlen(logErrBuf);  // reset current log file lenght
    		}

			// print logs in the files
    		fprintf(f[0], "%s", logOutBuf);
    		fflush(f[0]);
    		fprintf(f[1], "%s", logErrBuf);
    		fflush(f[1]);

            if (killed != 0) // normal process flow, process not killed by ^C
            {
            	if(i == cmds-1)  // print only the last command in case of piping
            	{
    	        	printf("%s", buf);  // print the stdout in the shell
            	}
            	printf("%s\n", buf2);  // print the stderr in the shell
            }
            killed = 1;  // restore default value

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

		// free dynamic arrays
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
	killed = kill(pid, SIGKILL);
}
