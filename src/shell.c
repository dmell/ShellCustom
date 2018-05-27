/*

main function of the project.

main inizializes the global variables, calls the check parameters function,
opens the log files, handles the prompt for the user and takes the input commands,
does a first parsing of the commands looking for multiple commands ( && || ;).
When "exit" command occours, main frees the memory and closes the streams

*/


#include "commons.h"
#include "shellutil.h"
#include "parsers.h"
#include "run.h"

int main(int argc, char **argv)
{
	// initialization of the global variables
	logOutLen = 0;  // current lenght of the log file for stdout
	logErrLen = 0;  // current lenght of the log file for stdout
	outfile = NULL;  // the name of log outfile
	errfile = NULL;  // the name of log errfile

	// lengths set to -1 to check if the user has specified new lengths
	// in the parameters, otherwise 4096 by default
	logfileLenght = -1;
	bufLenght = -1;

	code = 0;

	// global file descriptor for stdin, stdout, stderr, to restore the default
	// I/O stream whenever it is necessary
	stdin_restore = dup(0);
	stdout_restore = dup(1);
	stderr_restore = dup(2);

	killed = 1; // check if the process has been killed by ^C (1 = not yet killed)

	checkParameters(argc, argv);  // check command line parameters given by the user

	// open streams for the log (in writing mode) with the names given by the user
	FILE * fp[2];
	fp[0] = fopen(outfile, "w");
	// if the user wants both logs in the same file, we check the strings and then
	// without changing all the code we make the second file pointer point the same 
	// file. The flag is set to change behaviour in counting characters.
	if (strcmp(outfile, errfile) == 0)
	{
		fp[1] = fp[0];
		doubleLog = 0;
	}
	else
	{
		fp[1] = fopen(errfile, "w");
		doubleLog = 1;
	}

	if (fp[0] == NULL || fp[1] == NULL) // if fopen failed
	{
		perror("fopen");
		exit(1);
	}

	// objects for the getline function
	char *line = NULL;  // string for the command given by the user
	size_t len = 0;  // current lenght of line
	ssize_t read = 0;  // return value of getline, number of read character

	char ** cmd; // in every position there will be a string containing a command and its arguments.
	int cmds=1;  // there's always at least one command

    //Brief description of the Shell
    printf("CUSTOM SHELL - 185322, 186291, 186893\n");
    printf("Simple shell based on BASH with real time on file logging capabilities\n");
    printf("Type exit to dismiss\n");

	while (1) {
		fprintf(stdout, ">> ");  // print the prompt
		fflush(stdout);
		read = getline(&line, &len, stdin);  // read the commands given by the user

		// remove empty spaces before the command
		int index=0;
		while (line[index] == ' ')
		{
			index++;
		}
		if (index != 0)
		{
			line = substring(line, index, read);
		}

		if (strcmp(line, "\n") == 0)  // avoid to execute null command in case of empty line
		{
			continue;
		}
		if (strcmp("exit\n",line) == 0)  // handling of exit command
		{
			break;
		}

		// multiple commands
		// save here the sequence of the operators ; && ||. NB the last is an 'e' char to indicate the end
		char * operators = malloc(MAXOPERATORS*sizeof(char));
		bzero(operators, MAXOPERATORS);
		char ** commands;  // save here the commands divided by ; && ||

		 // parse the user input finding multiple commans, also modifing operators array
		commands = findMultipleCommands(&operators, line);

		int indexMultipleCommands = 0;
		int valuePreviousCommand = 0;  // used as boolean for the return value of run function

		// iterate on the various commands divided by && || ;
		do {
			if ((indexMultipleCommands == 0) ||  // the first command is always executed
			   // previous command is false and this command follows a || operator
			   ((operators[indexMultipleCommands-1] == '|') && (valuePreviousCommand == 0)) ||
			   // previous command is true and this command follows a && operator
			   ((operators[indexMultipleCommands-1] == '&') && (valuePreviousCommand == 1)) ||
			   // this command follows a ; operator
			   (operators[indexMultipleCommands-1] == ';'))
			{
				// parseCommand returns an array of commands divided by piping character
				cmd = parseCommand(commands[indexMultipleCommands], &cmds);

				// run execute the command and the logging
				valuePreviousCommand = run(cmd, cmds, fp);

				int i;
				// parseCommand allocates every time a new char **, we can free the memory
				for (i = 0; i < cmds; i++)
				{
					free(cmd[i]);
				}
				free(cmd);

				cmds = 1;
			}
			indexMultipleCommands++;
		} while (operators[indexMultipleCommands-1] != 'e');


		free(operators);
		int i;
	}

	// free dynamic allocation of strings
	free(line);
	free(outfile);
	free(errfile);

	// close streams
	if (fclose(fp[0]))
	{
		perror("fclose");
		exit(1);
	}
	if (doubleLog)
	{
		if (fclose(fp[1]))
		{
			perror("fclose");
			exit(1);
		}
	}

	
	printf("Goodbye\n");
	return 0;
}
