#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

char * estrai (char *source, int index);

int main(int argc, char **argv)
{
    // verifico se è stata richiesta la consultazione del manuale
    int man = 0; // usato come bool
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
        printf("\n\t\t\t\t\t\t(in number of characters) (5000 by default)\n");
        printf("  -c,--code\t\t\t\t\talso indicates the return code of the commands\n\n");
        exit(0);
    }

    // memorizzo parametri inseriti
    char *outfile = NULL;  // stringa con nome oufile di log
    char *errfile = NULL;  // stringa con nome errfile di log
    // lunghezza file di log (inizializzata a -1 per controllare inserimento utente,
    // poi settata a 5000 se l'utente non indica nulla)
    int logfileLenght = -1;
    int code = 0;  // usato come bool per opzione codice uscita
    for (int i = 1; i < argc; i++)
    {
        if (strncmp(argv[i], "-o=", 3) == 0)  // parametro outfile corto
        {
            if (outfile == NULL)  // outfile non ancora indicato
            {
                outfile = estrai (argv[i], 3);
            }
            else  // outfile già indicato
            {
                printf("shell: outfile parameter already entered.\n");
                printf("Try './shell --help' for more information.\n");
                exit(1);
            }
        }
        else if (strncmp(argv[i], "--outfile=", 10) == 0)  // parametro outfile lungo
        {
            if (outfile == NULL)  // outfile non ancora indicato
            {
                outfile = estrai (argv[i], 10);
            }
            else  // outfile già indicato
            {
                printf("shell: outfile parameter already entered.\n");
                printf("Try './shell --help' for more information.\n");
                exit(1);
            }
        }
        else if (strncmp(argv[i], "-e=", 3) == 0)  // parametro errfile corto
        {
            if (errfile == NULL)  // errfile non ancora indicato
            {
                errfile = estrai (argv[i], 3);
            }
            else  // errfile già indicato
            {
                printf("shell: errfile parameter already entered.\n");
                printf("Try './shell --help' for more information.\n");
                exit(1);
            }
        }
        else if (strncmp(argv[i], "--errfile=", 10) == 0)  // parametro errfile lungo
        {
            if (errfile == NULL)  // errfile non ancora indicato
            {
                errfile = estrai (argv[i], 10);
            }
            else  // errfile già indicato
            {
                printf("shell: errfile parameter already entered.\n");
                printf("Try './shell --help' for more information.\n");
                exit(1);
            }
        }
        else if (strncmp(argv[i], "-m=", 3) == 0)  // parametro maxlen corto
        {
            if (logfileLenght == -1)  // lunghezza non ancora indicata
            {
                logfileLenght = atoi(estrai (argv[i], 3));
            }
            else  // lunghezza già indicata
            {
                printf("shell: maxlen parameter already entered.\n");
                printf("Try './shell --help' for more information.\n");
                exit(1);
            }
        }
        else if (strncmp(argv[i], "--maxlen=", 9) == 0)  // parametro maxlen lungo
        {
            if (errfile == NULL)  // lunghezza non ancora indicata
            {
                logfileLenght = atoi(estrai (argv[i], 9));
            }
            else  // lunghezza già indicata
            {
                printf("shell: maxlen parameter already entered.\n");
                printf("Try './shell --help' for more information.\n");
                exit(1);
            }
        }


    }

    if (logfileLenght == -1)  // se l'utente non ha settato lunghezza massima file di log
    {
        logfileLenght = 5000;  // lunghezza di default
    }





    char *line = NULL;  // stringa dove viene memorizzato il comando inserito dall'utente
    size_t len = 0;  // ???
    ssize_t read = 0;  // numero di caratteri letti (valore di ritorno di getlineq)
    int error;  // codice di errore dell'esecuzione del comando (valore di ritorno di system)

    while (1) {
        printf(">> ");
        read = getline(&line, &len, stdin);
        error = system(line);
    }

    free(line);
    return 0;
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
