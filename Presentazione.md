---
author:
- 'Riccardo Micheletto - Matteo Strada - Damiano Melotti'
date: Maggio 2018
title: |
    Progetto di Sistemi Operativi\
    Traccia 1 - Custom Shell
...

Specifiche
==========

La prima traccia del progetto di Sistemi Operativi 1, dell’anno
accademico 2017/18 prevede lo sviluppo di una shell custom, in grado di
gestire i principali comandi da terminale, il piping e altre
funzionalità avanzate previste dalla bash di sistema. Inoltre, lo
*standard output* e lo *standard error* devono essere salvati
all’interno di due file di log, con un layout che indichi il comando che
ha generato tale output, la data e se l’utente lo richiede, il codice di
ritorno.

Soluzione
=========

Il nostro gruppo è partito concentrandosi sul funzionamento di base, con
l’obiettivo di soddisfare i requisiti minimi, per poi aggiungere mano a
mano funzionalità avanzate.

Argomenti
---------

La nostra Shell Custom prende i seguenti argomenti:

-   `-o=`, `–outfile=` per specificare il file di log per *stdout*;

-   `-e=`, `–errfile=` per specificare il file di log per *stderr*;

-   `-m=`, `–maxlen=` per limitare il numero di caratteri all’interno
    dei log;

-   `-s=`, `–size=` per limitare il numero di caratteri della risposta
    di ogni comando;

-   `-c`, `–code` per includere il codice di ritorno dei comandi;

-   `–help` per visualizzare il manuale d’uso.

Gli argomenti possono essere inseriti in qualsiasi ordine.

Funzionalità
------------

Tutti i comandi “standard” vengono eseguiti correttamente, tra cui i
vari `ls`, `date`, `cat` ecc, ed è stato implementato il supporto al
piping. Ogni comando viene dettagliatamente parsato e poi genera un
nuovo processo che lo esegue attraverso una system call della famiglia
`exec`.\
Inoltre, abbiamo aggiunto il supporto al redirezionamento e ai comandi
multipli (separati da &&, $\Vert$ e ;).\
I file di log presentano il nome del nome del comando in questione,
l’eventuale sottocomando in caso di piping, la data in cui è stato
eseguito e l’output (*stdout* o *stderr*). Addizionalmente l’utente può
richiedere il codice di ritorno con il flag sopraindicato. Nel caso di
comando sconosciuto, viene stampato un errore di sistema nel primo log,
mentre lo standard error nel secondo.

Osservazioni
============

-   Nel caso in cui un comando abbia dimensione dell’output maggiore del
    buffer indicato viene stampato un messaggio di avviso. In ogni caso
    tale valore viene controllato, e se inferiore ad una soglia di
    utilizzo minimo si stampa una segnalazione.

-   Se invece l’i-esimo comando supererebbe la dimensione di uno dei
    file di log con il suo output, la shell notifica l’utente il quale
    può scegliere mediante un menù interattivo se sovrascrivere il file,
    specificarne un nuovo oppure uscire dalla sessione.

-   La gestione dei comandi interattivi si è rivelata particolarmente
    scabrosa: nella versione finale si riescono ad intercettare segnali
    di interruzione soltanto in alcuni comandi (ad esempio `python` e
    `gdb`). Per quanto riguarda gli altri, tipo gli editor `nano` e
    `vim`, che intercettano le shortcut da tastiera utilizzando la
    libreria ncurses in modalità row, essendo molto complesso supportare
    tale libreria non abbiamo previsto supporto, per cui per uscirvi è
    necessario digitare la sequenza di tasti corrispondente (ad esempio
    per nano, Ctrl+X, n e invio).

Conclusione
===========

Ringraziamo il prof. Crispo e il dr. Naimoli per aver tenuto questo
corso, e ci auguriamo che il nostro lavoro venga apprezzato.\
\
\

  ------------------------------ --
  Micheletto Riccardo - 185322   
  Strada Matteo - 186291         
  Melotti Damiano - 186893       
  ------------------------------ --
