#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>

#include "strutture_dati/tipi_componente.h"
#include "strutture_dati/coda_stringhe.h"
#include "comunicazione/comunicazione.h"



/*
* Funzione che calcola il valore di un registro intero.
* Return: FALSE in caso di errori, TRUE altrimenti.
*/
boolean calcola_registro_intero( const registro* registro, int* res );

/*
* Funzione che calcola il valore di un registro stringa.
* Return: FALSE in caso di errori, TRUE altrimenti.
*/
boolean calcola_registro_stringa( const registro* registro, string res);


/*
* Funzione principale: sta in ascolto e interpreta i messaggi che arrivano.
*/
void ascolta_e_interpreta( registro* registri[], int numero_registri, boolean* accesa);

/*
* Funzione per gestire il messaggio LABELUP, aggiornamento di un interruttore.
*/
void gestisci_LABELUP( coda_stringhe* args, registro* registri[], boolean* stato );

/*
* Funzione per gestire il messaggio STATUSGET, per avere lo status.
*/
void gestisci_STATUSGET( coda_stringhe* args, registro* registri[], int numero_registri );

/*
* Funzione per gestire il messaggio ID, per sapere se un dato ID è il mio.
*/
void gestisci_ID(coda_stringhe* separata);

/*
* Funzione per la terminazione del processo.
*/
void termina(int x);

/*
* Indica quando è stata accesa.
*/
long accensione;

/*
* L'identificativo univoco del componente.
*/
int id;




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
  id = atoi(argv[1]);

  /*
  * E' possibile fornire anche gli altri valori al posto di quelli standard in questo
  * ordine: stato tempo_di_utilizzo.
  */
  if( argn > 3 ){
    if( strcmp(argv[2], "ON") ){
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
  signal(SIGINT, termina);

  /*
  * Sto perennemente in ascolto sulla mia pipe FIFO
  */
  while(1){

    ascolta_e_interpreta( registri, numero_registri, &accesa );

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


void ascolta_e_interpreta( registro* registri[], int numero_registri, boolean* accesa ){

  registro* tempo_utilizzo = registri[0];
  // Quando arriva un messaggio lo leggo e tolgo il \n finale, se presente.
  char messaggio[100];
  while( !leggi_messaggio(id, "/tmp", messaggio, 99))
    perror("Errore in lettura");
  //fgets(messaggio, 99, stdin);

  strtok(messaggio, "\n");

  // Creo la coda di stringhe.
  coda_stringhe* separata = crea_coda_da_stringa(messaggio, " ");

  // Recupero il nome del comando.
  char nome_comando[20];
  if(!primo(separata, nome_comando, TRUE) )
    exit(140);

  if( strcmp(nome_comando, GET_STATUS) == 0 ){

    gestisci_STATUSGET(separata, registri, numero_registri);

  } else if( strcmp(nome_comando, UPDATE_LABEL) == 0 ){

    gestisci_LABELUP(separata, registri, accesa);

  } else if( strcmp(nome_comando, REMOVE) == 0 ){

    char tmp[20];
    primo(separata, tmp, TRUE);
    int id_ric = atoi(tmp);
    if( id_ric == id || id_ric == ID_UNIVERSALE ){

      termina(0);

    }

  } else if( strcmp(nome_comando, ID) == 0 ){

    gestisci_ID(separata);

  } else {

    printf("Comando non supportato: %s\n", nome_comando);

  }

}


void gestisci_LABELUP( coda_stringhe* separata, registro* registri[], boolean* accesa){

  registro* tempo_utilizzo = registri[0];

  char id_comp[20];
  primo(separata, id_comp, TRUE);
  int id_ric = atoi(id_comp);

  if( id_ric == id || id_ric == ID_UNIVERSALE ){

    // Recupero il nome dell'interrutore, il nuovo stato e aggiorno
    char interruttore[20];
    primo(separata, interruttore, TRUE);

    if( strcmp(interruttore, "ACCENSIONE") == 0 ){

      char nuovo_stato[20];
      primo(separata, nuovo_stato, TRUE);

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
  } else
    distruggi_coda(separata);

}

void gestisci_STATUSGET( coda_stringhe* separata, registro* registri[], int numero_registri ){

    char indice_ric[10];
    primo(separata, indice_ric, TRUE);
    int indice = atoi(indice_ric);
    if( indice == ID_UNIVERSALE || indice == id ){
      int i = 0;
      char res[1024*2];
      sprintf(res, "%s BULB, id: %d ", GET_STATUS_RESPONSE, id );
      for( i = 0; i < numero_registri; i++ ){

        char str[1024];
        stampa_registro(registri[i], str);
        strcat(res, " ");
        strcat( res, str );

      }
      manda_messaggio(id, (string) PERCORSO_BASE_DEFAULT, res);
    }

}

void gestisci_ID(coda_stringhe* separata){

  char tmp[10];
  primo(separata, tmp, TRUE);
  if( atoi(tmp) == id )
    manda_messaggio(id, (string) PERCORSO_BASE_DEFAULT, "TRUE");
  else
    manda_messaggio(id, (string) PERCORSO_BASE_DEFAULT, "FALSE");

}

void termina(int x){

  close(file);
  char pipe[50];
  sprintf(pipe, "/tmp/%d", id);
  unlink(pipe);
  exit(0);

}
