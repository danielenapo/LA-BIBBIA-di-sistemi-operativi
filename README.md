# LA-BIBBIA-di-sistemi-operativi
le sacre scritture della parte in C dell'esame di Sistemi Operativi.

## makefile
```makefile

CC=gcc
CFLAGS=-Wall
BIN=main

all: $(BIN)

%:	%.c
	$(CC) $(CFLAGS) -o $@ $<

```

## COMMENTI OBBLIGATORI:
#### (o almeno credo, sono quelli che mette sempre la prof)

  * sulle variabili (quando si definiscono all'inizio)
  * per l'inizio del figlio (quando c'è if(pid==0))
  * commento che dice che i figli tornano -1 in caso di errore (appena inizia il figlio)
  * spiega lo schema di comunicazione (prima di chiudere le pipe inutilizzate)
  
  
