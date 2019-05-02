#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "strutture_dati/tipi_componente.h"
#include "strutture_dati/coda_stringhe.h"
#include "comunicazione/comunicazione.h"



/*
* Funzione che calcola il valore di un registro intero.
* Return: FALSE in caso di errori, TRUE altrimenti.
*/
boolean calcola_registro_intero( const registro* registro, int* res );

/*
* Funzione principale: sta in ascolto e interpreta i messaggi che arrivano.
*/
void ascolta_e_interpreta( int* id, registro* registri[], int numero_registri, boolean* accesa);

/*
* Funzione per gestire il messaggio LABELUP, aggiornamento di un interruttore
*/
void gestisci_LABELUP( coda_stringhe* args, registro* registri[], boolean* stato );

void gestisci_STATUSGET( coda_stringhe* args, registro* registri[], int numero_registri, int id);

/*
* Funzione per la terminazione del processo.
*/
void termina(int x);

/*
* Indica quando è stata accesa.
*/
long accensione;

int main( int argn, char** argv ){

  /*
  * Valore booleano, indica se è accesa o no.
  */
  boolean accesa = FALSE;

  /*
  * Registto "time"
  */
  registro tempo_utilizzo;
  strcpy(tempo_utilizzo.nome, "time");
  tempo_utilizzo.da_calcolare = FALSE;
  tempo_utilizzo.valore.integer = 0;
  tempo_utilizzo.is_intero = TRUE;

  int numero_registri = 1;
  registro* registri[] = {&tempo_utilizzo};


  if( argn < 2 )
    exit(130);

  /*
  * L'id del componente. deve essere fornito come primo argomento sulla
  * linea di comando.
  */
  int id;
  id = atoi(argv[1]);

  printf("id: %d\n", id);


  /*
  * E' possibile fornire anche gli altri valori al posto di quelli standard in questo
  * ordine: stato tempo_di_utilizzo.
  */
  if( argn > 3 ){
    if( strcmp(argv[2], "on") ){
      accensione = (long) time(NULL);
      tempo_utilizzo.da_calcolare = TRUE;
      accesa = TRUE;
    }
    else
      accesa = FALSE;
  }

  if( argn > 4 ){
    string tempo = argv[3];
    tempo_utilizzo.valore.integer = atoi(tempo);
  }

  crea_pipe(id, (string) PERCORSO_BASE_DEFAULT);

  /*
  * Sto perennemente in ascolto sulla mia pipe FIFO
  */

  int i;
  for( i = 0; i < numero_registri; i++ ){

    char stampa[100];
    stampa_registro(registri[i], stampa);
    printf(" >> %s\n", stampa);

  }

  while(1){

    ascolta_e_interpreta( &id, registri, numero_registri, &accesa );

  }

}

boolean calcola_registro_stringa( const registro* registro, string res ){

  return TRUE;

}

boolean calcola_registro_intero( const registro* registro, int* res ){

  if( registro -> da_calcolare == FALSE || registro -> is_intero == FALSE )
    return FALSE;
  if( strcmp(registro -> nome, "time") == 0 ){

    long ora = (long) time(NULL);
    *res = (registro -> valore.integer + (ora) - accensione);

  }
  return TRUE;
}


void ascolta_e_interpreta( int* id, registro* registri[], int numero_registri, boolean* accesa ){

  registro* tempo_utilizzo = registri[0];
  // Quando arriva un messaggio lo leggo e tolgo il \n finale, se presente.
  char messaggio[100];
  while( !leggi_messaggio(*id, "/tmp", messaggio, 99))
    perror("Errore in lettura");
  //fgets(messaggio, 99, stdin);

  printf("Ricevuto qualcosa: %s\n", messaggio);
  strtok(messaggio, "\n");

  // Creo la coda di stringhe.
  coda_stringhe* separata = crea_coda_da_stringa(messaggio, " ");

  // Recupero il nome del comando.
  char nome_comando[20];
  if(!primo(separata, nome_comando, TRUE) )
    exit(140);

  if( strcmp(nome_comando, "STATUSGET") == 0 ){

    gestisci_STATUSGET(separata, registri, numero_registri, *id);

  } else if( strcmp(nome_comando, "LABELUP") == 0 ){

    gestisci_LABELUP(separata, registri, accesa);

  } else if( strcmp(nome_comando, "REMOVE") == 0 ){

    exit(0);

  } else {

    printf("Comando non supportato: %s\n", nome_comando);

  }

}


void gestisci_LABELUP( coda_stringhe* separata, registro* registri[], boolean* accesa){

  registro* tempo_utilizzo = registri[0];
  // Recupero il nome dell'interrutore, il nuovo stato e aggiorno
  char interruttore[20];
  primo(separata, interruttore, TRUE);

  if( strcmp(interruttore, "ACCENSIONE") == 0 ){

    char nuovo_stato[20];
    primo(separata, nuovo_stato, TRUE);

    printf("[LABELUP]: %s\n", nuovo_stato);
    if( strcmp(nuovo_stato, "ON") == 0 ){
      if( *accesa == FALSE ){
        accensione = (long) time(NULL);
        tempo_utilizzo -> da_calcolare = TRUE;
      }
      *accesa = TRUE;

    }
    else {

      if( *accesa == TRUE ){
        long ora = (long) time(NULL);
        tempo_utilizzo -> valore.integer += (ora - accensione);
        tempo_utilizzo -> da_calcolare = FALSE;
      }
      *accesa = FALSE;
    }

  }

}

void gestisci_STATUSGET( coda_stringhe* separata, registro* registri[], int numero_registri, int id ){

    char indice_ric[10];
    primo(separata, indice_ric, TRUE);
    int i = 0;
    char res[1024*2];
    sprintf(res, "STATUSGETRES BULB, id: %d ", id );
    for( i = 0; i < numero_registri; i++ ){

      char str[1024];
      stampa_registro(registri[i], str);
      strcat(res, " ");
      strcat( res, str );

    }
    manda_messaggio(atoi(indice_ric), (string) PERCORSO_BASE_DEFAULT, res);x

}
