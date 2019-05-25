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
* Funzione per gestire il messaggio STATUSGET, per avere lo stato.
*/
void gestisci_STATUSGET( coda_stringhe* args, registro* registri[], int numero_registri, boolean* accesa );

/*
* Funzione per gestire il messaggio ID, per sapere se un dato ID è il mio.
*/
void gestisci_ID(coda_stringhe* separata);

/*
* Funzione per la terminazione del processo.
*/
void termina(int x);

/*
* Funzione che genera i processi per stare in ascolto sulla FIFO esterna ( per l'umano )
* e per stare in ascolto sulla FIFO interna alla lampadina.
*/
void crea_processi_supporto(registro* registri[], int numero_registri, boolean* accesa);

/*
* Indica quando è stata accesa.
*/
long accensione;

/*
* L'identificativo univoco del componente.
*/
int id;

char pipe_interna[50]; //pipe per comunicare all'interno della lampadina
char pipe_esterna[50]; //pipe per comunicare con l'umano

pid_t figli[2];//memorizzo i process_id dei figli (i processi che costituiscono la lampadina)

char* a;

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

  //gestisco gli input
  if( argn < 2 ) //se ricevo meno di due argomenti restituisco messaggio di errore
    exit(130);

  /*
  * L'id del componente. deve essere fornito come primo argomento sulla
  * linea di comando.
  */
  id = strtol(argv[1], &a, 10);
  if( id == 0 )
    exit(0);

  /*
  * E' possibile fornire anche gli altri valori al posto di quelli standard in questo
  * ordine: stato tempo_di_utilizzo.
  * FORMATO INPUT : nome_file id stato tempo_di_utilizzo
  */
  if( argn >= 3 ){ //recupero lo stato
    if( strcmp(argv[2], "ON") == 0 ){ //se devo ACCENDERE la lampadina
      accensione = (long) time(NULL); //aggiorno il registro time
      tempo_utilizzo.da_calcolare = TRUE;
      accesa = TRUE; //aggiorno lo stato (= "accendo la lampadina")
    }
    else //se devo SPEGNERE la lampadina
      accesa = FALSE; //aggiorno lo stato (= "spengo la lampadina")
  }

  if( argn >= 4 ){ //recupero il tempo di utilizzo
    string tempo = argv[3]; //aggiorno il registro time
    tempo_utilizzo.valore.integer = strtol(tempo, &a, 10);
  }

  /* creo le pipe destinate alla comunicazione con l'umano (pipe_esterna)
  *  e con gli altri processi della lampadina (pipe_interna)
  */
  sprintf(pipe_interna, "%s/%d_int", (string) PERCORSO_BASE_DEFAULT, id);
  sprintf(pipe_esterna, "%s/%d_ext", (string) PERCORSO_BASE_DEFAULT, id);

  /*
  * Sto perennemente in ascolto sulla mia pipe FIFO
  */

  crea_processi_supporto(registri, numero_registri, &accesa);


}

boolean calcola_registro_stringa( const registro* registro, string res ){

  return TRUE;

}

boolean calcola_registro_intero( const registro* registro, int* res ){
  //se il registro non è da calcolare o non è intero restituisco FALSE
  if( registro -> da_calcolare == FALSE || registro -> is_intero == FALSE )
    return FALSE;
  //se il registro si chiama time, ne aggiorno il valore e restituisco TRUE
  if( strcmp(registro -> nome, "time") == 0 ){

    long ora = (long) time(NULL);
    *res = (registro -> valore.integer + (ora) - accensione);

  }
  return TRUE;
}


void ascolta_e_interpreta( registro* registri[], int numero_registri, boolean* accesa ){


  registro* tempo_utilizzo = registri[0];
  // Quando arriva un messaggio lo leggo e tolgo il "\n" finale, se presente.
  char messaggio[100];
  //se riscontro un errore in lettura mandi messaggio all'utente
  while( read_msg(pipe_interna, messaggio, 99) == FALSE)
    perror("Errore in lettura");


  strtok(messaggio, "\n"); //elimino "\n" finale

  //printf("[BULB]%s\n",messaggio );
  // Creo la coda di stringhe.
  coda_stringhe* separata = crea_coda_da_stringa(messaggio, " ");


  // Recupero il nome del comando.
  char nome_comando[20];
  if(!primo(separata, nome_comando, FALSE) ) //se non riesco a leggere esco
    exit(140);

  if( strcmp(nome_comando, GET_STATUS) == 0 ){ //comando che chiede lo STATO

    gestisci_STATUSGET(separata, registri, numero_registri, accesa);

  } else if( strcmp(nome_comando, UPDATE_LABEL) == 0 ){ //comando per AGGIORNARE un INTERRUTTORE

    gestisci_LABELUP(separata, registri, accesa);

  } else if( strcmp(nome_comando, REMOVE) == 0 ){ //comando per rimuovere un dispositivo

    char tmp[20];
    primo(separata, tmp, FALSE);
    int id_ric = strtol(tmp, &a, 10);
    if( id_ric == id || id_ric == ID_UNIVERSALE ){ //se sono io il dispositivo da rimuovere

      termina(0);

    }
    send_msg(pipe_interna, "TRUE");

  } else if( strcmp(nome_comando, ID) == 0 ){ //comando per sapere se il dispositivo con l'ID inviato sia raggiungibile attraverso la lampadina

    gestisci_ID(separata);

  } else if( strcmp(nome_comando, "CONFIRM") == 0 ){ //comando per verificare se l'ID passato sia quello della lampadina

    gestisci_ID(separata);


  } else{ //qualsiasi altro comando non riconosciuto

    send_msg(pipe_interna, "DONE");

  }
  distruggi(separata);

}


void gestisci_LABELUP( coda_stringhe* separata, registro* registri[], boolean* accesa){

  registro* tempo_utilizzo = registri[0];

  char id_comp[20];
  primo(separata, id_comp, FALSE); //recupero l'id del componente interessato
  int id_ric = strtol(id_comp, &a, 10);
  boolean res = FALSE;
  //se l'id è il mio (= sono io a dover eseguire il comando)
  if( id_ric == id || id_ric == ID_UNIVERSALE ){

    // Recupero il nome dell'interrutore, il nuovo stato e aggiorno
    char interruttore[20];
    primo(separata, interruttore, FALSE); //recupero il nome dell'interruttore da aggiornare

    if( strcmp(interruttore, "ACCENSIONE") == 0 ){ //interruttore ACCENSIONE

      char nuovo_stato[20];
      primo(separata, nuovo_stato, FALSE); //recupero il nuovo stato dell'interruttore

      if( strcmp(nuovo_stato, "ON") == 0 ){ //se devo ACCENDERE la lampadina
        if( *accesa == FALSE ){ //se la lampadina è spenta
          accensione = (long) time(NULL); //aggiorno il registro time
          tempo_utilizzo -> da_calcolare = TRUE;
        }
        *accesa = TRUE; //aggiorno lo stato (= "accendo la lampadina")

      }
      else { //se devo SPEGNERE la lampadina

        if( *accesa == TRUE ){ //se è accesa
          long ora = (long) time(NULL); //aggiorno il registro time
          tempo_utilizzo -> valore.integer += (ora - accensione); //memorizzo il tempo che è rimasta accesa
          tempo_utilizzo -> da_calcolare = FALSE;
        }
        *accesa = FALSE; //aggiorno lo stato (= "spengo la lampadina")
      }
      res = TRUE;

    }
  }
  //informo gli altri processi che costituiscono la lampadina che ho terminato
  if( res == TRUE )
    send_msg(pipe_interna, "TRUE");
  else
    send_msg(pipe_interna, "FALSE");
}

void gestisci_STATUSGET( coda_stringhe* separata, registro* registri[], int numero_registri, boolean* accesa ){

    char indice_ric[10];
    primo(separata, indice_ric, FALSE); //recupero l'indice
    int indice = strtol(indice_ric, &a, 10);
    //se sono il dispositivo che deve eseguire il comando
    if( indice == ID_UNIVERSALE || indice == id ){
      int i = 0;
      char res[1024*2];
      //preparo il messaggio di risposta contenente il mio stato
      sprintf(res, "%s bulb %d %s", GET_STATUS_RESPONSE, id, *accesa == TRUE ? "ON" : "OFF" );
      for( i = 0; i < numero_registri; i++ ){ //stampa il valore del registro

        char str[1024];
        stampa_registro(registri[i], str);
        strcat(res, " ");
        strcat( res, str );

      }
      send_msg(pipe_interna, res); //invio la risposta con lo stato alla piep interna
    } else
      send_msg(pipe_interna, "DONE"); //informo i processi della lampadina di aver concluso l'azione

}

void gestisci_ID(coda_stringhe* separata){

  char tmp[10];
  primo(separata, tmp, FALSE); //recupero l'id
  if( strtol(tmp, &a, 10) == id ) //se corrisponde al mio
    send_msg(pipe_interna, "TRUE"); //restituisco TRUE
  else //se non corrisponde al mio
    send_msg(pipe_interna, "FALSE"); //restituisco FALSE

}

void termina(int x){

  // Uccido i miei figli.
  kill(figli[0], SIGKILL);
  kill(figli[1], SIGKILL);

  // Chiudo il file descriptor.
  close(file);

  // Distruggo tutte le FIFO create.
  ripulisci(id, (string) PERCORSO_BASE_DEFAULT);
  exit(0);

}

void crea_processi_supporto(registro* registri[], int numero_registri, boolean* accesa){

  crea_pipe(id, (string) PERCORSO_BASE_DEFAULT); //creo pipe per comunicare con il processo padre

  pid_t pid = fork(); //genero un processo identico a me
  if( pid == 0 ){ //se sono il filgio

    // Se sono il figlio, sto in ascolto perennemente sulla pipe dell'umano
    // e invio quello che ricevo sulla pipe interna.
    while(1){
      char msg[200];
      read_msg(pipe_esterna, msg, 200);
      sem_wait(sem);
      send_msg(pipe_interna, msg);
      read_msg(pipe_interna, msg, 199);
      sem_post(sem);
    }

  } else if( pid > 0 ){ //se sono il padre

    // Il padre crea un nuovo figlio.
    figli[0] = pid; //mi salvo il process-id del figlio generato
    pid = fork(); //genero un altro figlio
    if( pid == 0 ){ //se sono il figlio
      // Il padre sta in ascolto sulla PIPE del controllore del componente e invia
      // tutto quello che riceve sulla FIFO interna al componente, aspettando un messaggio
      // DONE o altro e in caso lo restituisce al padre.
      while(1){
        char msg[200];
        leggi_messaggio(id, (string) PERCORSO_BASE_DEFAULT, msg, 199); //legge messaggio da pipe con il padre
        sem_wait(sem);
        send_msg(pipe_interna, msg); //scrive sulla pipe interna il messaggio preso dal padre
        read_msg(pipe_interna, msg, 199); //legge dalla pipe interna
        if( strcmp(msg, "DONE") != 0 ) //se riceve "DONE" non fa nulla, qualsiasi altra cosa viene inviata sulla pipe con il padre
          manda_messaggio(id, (string) PERCORSO_BASE_DEFAULT, msg);
        sem_post(sem);
      }

    } else if( pid > 0 ){ //se sono il padre

      // Il figlio sta in ascolto sulla FIFO interna e interpreta i comandi che arrivano.
      figli[1] = pid; //salvo il process_id del figlio generato
      signal(SIGINT, termina);//se ricevo un segnale per terminare, termino
      while(1){
        //gestisco i messaggi ch emi arrivano
        ascolta_e_interpreta(registri, numero_registri, accesa);
      }

    }

  }

}
