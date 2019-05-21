#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "strutture_dati/tipi_componente.h"
#include "strutture_dati/coda_stringhe.h"
#include "comunicazione/comunicazione.h"

/*
* L'id univoco del componente.
*/
int id;

/*
* Lista che contiene le pipes dei figli.
*/
lista_stringhe* lista_pipes;
lista_stringhe* tipi_figli;
lista_stringhe* stati_attesi;

/*
* pipe_interna: FIFO per le comunicazioni interne al processo.
* pipe_esterna: FIFO per le comunicazioni con l'umano.
*/
char pipe_interna[50];
char pipe_esterna[50];

/*
* I process ID dei processi di supporto figli.
*/
pid_t figli[2];

/*
* Funzione che calcola il valore di un registro intero.
* r: registro da calcolare
* res: variabile in cui verrà salvato il valore.
* Return: FALSE in caso di errori, TRUE altrimenti.
*/
boolean calcola_registro_intero( const registro* r, int* res );

/*
* Funzione che calcola il valore di un registro di tipo stringa.
* r: registro da calcolare
* output: variabile in cui verrà salvato il valore.
* Return: FALSE in caso di errori, TRUE altrimenti.
*/
boolean calcola_registro_stringa( const registro* r, string output);

/*
* Funzione che sta sempre in ascolto sulla FIFO interna ed interpreta i comandi.
*/
void ascolta_e_interpreta();

/*
* Fuzione che gestisce il messaggio per avere li stato.
*/
void gestisci_STATUSGET(coda_stringhe* separata);

/*
* Funzione che gestisce il comando per la verifica dell'ID.
*/
void gestisci_ID(coda_stringhe* separata);

/*
* Funzione che gestisce il comando per generare nuovi figli.
*/
void gestisci_SPAWN(coda_stringhe* separata);

/*
* Funzione che gestisce il comando per rimuovere un componente
*/
void gestisci_REMOVE(coda_stringhe* separata);

/*
* Funzione che gestisce il comando per l'aggiornamento di un interruttore.
*/
void gestisci_LABELUP(coda_stringhe* separata);

void genera_figlio(coda_stringhe* separata);

boolean calcola_override(string str, lista_stringhe* tipi_figli, lista_stringhe* confronti);
void aggiorna_stati(string str);

/*
* Funzione per terminare il processo quando arriva un SIGINT.
*/
void termina(int x);

void decodifica_hub(string str);

/*
* Funzione che crea i processi per gestire le più pipe.
*/
void crea_processi_supporto();

int main( int argn, char** argv ){

  /*
  * Leggo l'id dalla riga fi comando.
  */
  if( argn < 2 )
    exit(130);
  id = atoi(argv[1]);

  lista_pipes = crea_lista();
  append(lista_pipes, "/tmp/10");

  tipi_figli = crea_lista();
  stati_attesi = crea_lista();

  /*
  * Leggo gli stati degli eventuali figli che devo creare.
  */
  int i;
  for( i = 4; i < argn-1; i++ ){

    printf("[HUB]%s\n", argv[i]);
    int count = 0;
    int j;
    decodifica_hub(argv[i]);
    printf("[HUB-EDIT]%s\n", argv[i]);
    coda_stringhe* coda = crea_coda_da_stringa(argv[i], " ");
    genera_figlio(coda);

  }

  /*
  * Inizializzo la lista delle PIPE dei miei figli.
  */


  /*
  * Creo le variabili che contengono i percorsi delle PIPE.
  */
  sprintf(pipe_interna, "%s/%d_int", (string) PERCORSO_BASE_DEFAULT, id);
  sprintf(pipe_esterna, "%s/%d_ext", (string) PERCORSO_BASE_DEFAULT, id);

  crea_processi_supporto();


}

void ascolta_e_interpreta(){

  // Leggo il messaggio.
  char messaggio_in[200];
  read_msg(pipe_interna, messaggio_in, 199);
  strtok(messaggio_in, "\n");

  // Divido la stringa per gli spazi.
  coda_stringhe* separata = crea_coda_da_stringa(messaggio_in, " ");

  char comando[50];
  primo(separata, comando, TRUE);

  if( strcmp( comando, GET_STATUS ) == 0 ){

    gestisci_STATUSGET(separata);

  } else if( strcmp(comando, UPDATE_LABEL) == 0 ){

    gestisci_LABELUP(separata);

  } else if( strcmp( comando, ID ) == 0 ) {

    gestisci_ID(separata);

  } else if( strcmp(comando, REMOVE) == 0 ){

    gestisci_REMOVE(separata);

  } else if( strcmp(comando, "SPAWN") == 0 ){

    gestisci_SPAWN(separata);

  }  else if( strcmp(comando, "CONFIRM") == 0 ){

    char tmp[20];
    primo(separata, tmp, TRUE);
    int d = atoi(tmp);
    if( d == id || d == ID_UNIVERSALE )
      send_msg(pipe_interna, "TRUE");
    else
      send_msg(pipe_interna, "FALSE");

  } else {

    printf("Comando non supportato\n");
    send_msg(pipe_interna, "DONE");

  }

}

boolean calcola_registro_intero( const registro* r, int* res ){

  return TRUE;

}

boolean calcola_registro_stringa( const registro* r, string output){

  return TRUE;

}

void termina(int x){

  // Uccido i miei figli
  kill(figli[0], SIGKILL);
  kill(figli[1], SIGKILL);

  // Chiudo il file descriptor.
  close(file);
  char path[200];
  sprintf(path, "%s/%d", (string) PERCORSO_BASE_DEFAULT, id);

  // Invio a tutti i miei figli il comando REMOVE
  nodo_stringa* it = lista_pipes -> testa;
  while( it != NULL ){

    string pipe = it -> val;
    char msg[50];
    sprintf(msg, "%s %d", REMOVE, ID_UNIVERSALE);
    send_msg(pipe, msg);
    it = it -> succ;

  }

  // Distruggo tutte le pipe.
  unlink(path);
  unlink(pipe_esterna);
  unlink(pipe_interna);
  exit(0);

}

void crea_processi_supporto(){

  pid_t pid = fork();
  if( pid == 0 ){

    // Se sono il figlio sto perennemente in ascolto sulla pipe con l'umano
    // e scrivo tutto su quella interna
    mkfifo(pipe_esterna, 0666);
    while(1){

      char msg[200];
      read_msg(pipe_esterna, msg, 200);
      if( msg[0] == 'H' )
        send_msg(pipe_interna, msg+1);

    }

  } else if( pid > 0 ){

    figli[0] = pid;
    pid = fork();
    if( pid == 0 ){

      // Se sono il figlio leggo dalla pipe con il controllore e invio su quella
      // interna, aspettando la risposta.
      while(1){
        crea_pipe(id, (string) PERCORSO_BASE_DEFAULT);
        char msg[200];
        leggi_messaggio(id, (string) PERCORSO_BASE_DEFAULT, msg, 199);
        printf("RICEVUTO: %s\n", msg);
        send_msg(pipe_interna, msg);
        read_msg(pipe_interna, msg, 199);
        if( strcmp(msg, "DONE") != 0 )
          manda_messaggio(id, (string) PERCORSO_BASE_DEFAULT, msg);
      }

    } else if( pid > 0 ) {

      // Se sono il padre sto in ascolto sulla pipe interna e interpreto i messaggi.
      figli[1] = pid;
      signal(SIGINT, termina);
      mkfifo(pipe_interna, 0666);
      while(1){
        ascolta_e_interpreta();
      }

    }

  }

}

void gestisci_STATUSGET(coda_stringhe* separata){

  char id_ric[50];
  primo(separata, id_ric, TRUE);
  int id_comp = atoi(id_ric);

  if( id_comp == id || id_comp == ID_UNIVERSALE ){

    // Creo il messaggio contenente la risposta.
    char* response = (char*) malloc(sizeof(char) * 200 * lista_pipes -> n);
    string my_status = (char*) malloc(sizeof(char) * 200 * lista_pipes -> n);
    sprintf(my_status, "%s hub %d" ,GET_STATUS_RESPONSE, id);


    // Mando a tutti i miei figli che voglio lo stato.
    nodo_stringa* it = lista_pipes -> testa;

    boolean override = FALSE;
    while( it != NULL ){

      string pipe = it -> val;
      char msg[200];
      sprintf(msg, "%s %d", GET_STATUS, ID_UNIVERSALE );
      if( send_msg(pipe, msg) == FALSE || read_msg(pipe, msg, 199) == FALSE  ){

        nodo_stringa* tmp = it;
        rimuovi_nodo(lista_pipes, it);
        free(tmp);


      } else {

        strcat(response, ",");
        int i = 0;
        char copia[1024];
        strcpy(copia, msg+strlen(GET_STATUS_RESPONSE)+1);
        if( override == FALSE )
          override = calcola_override(copia, tipi_figli, stati_attesi );
        for( i = 0; msg[i] != '\0'; i++ )
          if( msg[i] == ' ')
            msg[i] = '_';
        strcat(response, msg+strlen(GET_STATUS_RESPONSE)+1);

      }
      it = it -> succ;


    }
    // Rispondo sulla pipe_interna.
    char tmp[20];
    sprintf(tmp, " %s [", override == TRUE ? "TRUE":"FALSE" );
    strcat(my_status, tmp);
    strcat(my_status, response);
    strcat(my_status, " ]");
    send_msg(pipe_interna, my_status);
    free(response);

  } else {

    char msg[200];
    sprintf(msg, "%s %s", ID, id_ric);
    nodo_stringa* it = lista_pipes -> testa;
    boolean trovato = FALSE;
    while( it != NULL && trovato == FALSE ){

      char res[10];
      if( send_msg(it -> val, msg) == FALSE || read_msg(it -> val, res, 9) == FALSE ){

        nodo_stringa* l = it;
        rimuovi_nodo(lista_pipes,it);
        it = it -> succ;
        free(it);

      } else if( strcmp(res, "TRUE") == 0 ){

        trovato = TRUE;

      } else {

        it = it -> succ;

      }

    }

    if( trovato == TRUE ){

      char status[1024];
      sprintf(msg, "%s %s", GET_STATUS, id_ric);
      send_msg(it -> val, msg);
      read_msg(it -> val, status, 1023);
      send_msg(pipe_interna, status);

    } else {

      send_msg(pipe_interna, "DONE");

    }

  }

}

void gestisci_LABELUP(coda_stringhe* separata){

  char id_ric[50];
  primo(separata, id_ric, TRUE);
  int id_comp = atoi(id_ric);
  char msg[200];
  if( id_comp == id || id_comp == ID_UNIVERSALE ){

    // Prendo il nome dell'interruttore e la nuova posizione.
    char label[50];
    primo(separata, label, TRUE);
    char pos[50];
    primo(separata, pos, TRUE);

    // Invio il messaggio a tutti i miei figli.
    sprintf(msg, "%s %d %s %s", UPDATE_LABEL, ID_UNIVERSALE, label, pos);
    printf("[HUB MSG]%s\n", msg);


  } else {

    sprintf(msg, "%s %s", UPDATE_LABEL, id_ric);
    char tmp[20];
    while( primo(separata, tmp, TRUE) == TRUE ){
      strcat(msg, " ");
      strcat(msg, tmp);
    }

  }

  nodo_stringa* it = lista_pipes -> testa;
  while( it != NULL ){

    string pipe = it -> val;
    if( send_msg(pipe, msg) == FALSE ){

      nodo_stringa* tmp = it;
      rimuovi_nodo(lista_pipes, it);
      it = it -> succ;
      free(tmp);

    } else if( id_comp == id || id_comp == ID_UNIVERSALE ){


      char tmp[200];
      sprintf(tmp, "%s %d", GET_STATUS, ID_UNIVERSALE);
      send_msg(pipe, tmp);
      char stato[1024];
      read_msg(pipe, stato, 1023);
      aggiorna_stati(stato+strlen(GET_STATUS_RESPONSE)+1);
      it = it -> succ;

    } else
      it = it -> succ;

  }
  send_msg(pipe_interna, "DONE");

}

void gestisci_ID(coda_stringhe* separata){

  // Recupero l'ID e rispondo se è il mio o no.
  char id_ric[20];
  primo(separata, id_ric, TRUE);
  int id_comp = atoi(id_ric);
  if( id_comp == id || id_comp == ID_UNIVERSALE )
    send_msg(pipe_interna, "TRUE");
  else{

    char msg[20];
    sprintf(msg, "%s %s", ID, id_ric);
    nodo_stringa* it = lista_pipes -> testa;
    boolean flag = FALSE;
    while( it != NULL && flag == FALSE ){

      char res[10];
      if( send_msg(it -> val, msg) == FALSE || read_msg(it -> val, res, 9) == FALSE ){

        nodo_stringa* tmp = it;
        rimuovi_nodo(lista_pipes, it);
        it = it -> succ;
        free(tmp);

      } else if( strcmp(res, "TRUE") == 0 ){
        flag = TRUE;
      } else{
        it = it -> succ;
      }

    }
    if( flag == TRUE )
      send_msg(pipe_interna, "TRUE");
    else
      send_msg(pipe_interna, "FALSE");

  }
}

void gestisci_REMOVE(coda_stringhe* separata){

  // Recupero l'ID e in caso mi termino.
  char id_ric[20];
  primo(separata, id_ric, TRUE);
  int id_comp = atoi(id_ric);
  if( id_comp == id || id_comp == ID_UNIVERSALE )
    termina(0);
  else{


    char remove_msg[200], confirm_msg[50];
    sprintf(remove_msg, "%s %s", REMOVE, id_ric);
    sprintf(confirm_msg, "%s %s", "CONFIRM", id_ric);
    nodo_stringa* it = lista_pipes -> testa;

    while( it != NULL ){

      printf("[HUB PIPE]%s\n", it -> val);

      char res[10];
      if( send_msg( it -> val, confirm_msg ) == FALSE || read_msg(it -> val, res, 9) == FALSE ){

        nodo_stringa* l = it;
        rimuovi_nodo(lista_pipes, it);
        it = it -> succ;
        free(l);

      } else if( strcmp(res, "TRUE") == 0 ){

        send_msg(it -> val, remove_msg);
        nodo_stringa* l = it;
        rimuovi_nodo(lista_pipes, it);
        it = it -> succ;
        free(l);

      } else{
        printf("[RES-CONFIRM]%s\n", res);
        send_msg(it -> val, remove_msg);
        it = it -> succ;
      }

    }
    send_msg(pipe_interna, "DONE");

  }

}

void gestisci_SPAWN(coda_stringhe* separata){


  printf("entrato\n");
  char id_ric[20];
  primo(separata, id_ric, TRUE);
  int id_comp = atoi(id_ric);
  if( id_comp == id || id_comp == ID_UNIVERSALE ){

    genera_figlio(separata);

  }

  send_msg(pipe_interna, "DONE");

}

void genera_figlio(coda_stringhe* separata){

  char tmp[40], percorso[50];
  primo(separata, tmp, TRUE);
  sprintf(percorso, "./%s.out", tmp);


  pid_t pid = fork();

  if( pid == 0 ){

    char* params[] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
    // Se sono il figlio cambio l'immagine.

    params[0] = (char*) malloc(sizeof(char)*40);
    strcpy(params[0], percorso);

    primo(separata, tmp, TRUE);

    params[1] = (char*) malloc(sizeof(char)*40);
    strcpy(params[1], tmp);

    int i = 2;
    while( primo(separata, tmp, TRUE) ==  TRUE ){

      params[i] = (char*) malloc((strlen(tmp)+1) * sizeof(char));
      strcpy(params[i], tmp);
      i++;
    }

    execv(params[0], params);

  } else if( pid > 0 ) {

    // Se sono il padre aggiungo alla mia lista di pipes la pipe del figlio appena creato.
    char id[30];
    primo(separata, tmp, FALSE);
    char pipe_figlio[100];
    sprintf(pipe_figlio, "%s/%s", (string) PERCORSO_BASE_DEFAULT, tmp);
    append(lista_pipes, pipe_figlio);
    distruggi(separata);

  }

}

boolean calcola_override(string str, lista_stringhe* tipi_figli, lista_stringhe* confronti){

  printf("[STRING]%s\n", str );
  boolean res = TRUE;
  char copia[1024];
  strcpy(copia, str);
  coda_stringhe* coda = crea_coda_da_stringa(str, " ");

  char tipo[20];
  primo(coda, tipo, FALSE);

  printf("[TIPO]%s\n", tipo);

  if( strcmp(tipo, "hub") == 0 || strcmp(tipo, "timer") == 0 ){

    decodifica_hub(copia);
    coda_stringhe* figli = crea_coda_da_stringa(copia, " ");
    char stato[400];
    primo(figli, stato, TRUE);
    primo(figli, stato, TRUE);
    primo(figli, stato, TRUE);
    primo(figli, stato, TRUE);
    while(primo(figli, stato, TRUE) == TRUE )
      if( strcmp(stato, "]") != 0)
        res = calcola_override(stato, tipi_figli, confronti);
    distruggi(coda);

  } else {

    printf("[ELSE]\n");
    nodo_stringa* it = tipi_figli -> testa;

    char confronto[20];
    primo(coda, confronto, FALSE);
    primo(coda, confronto, FALSE);

    int i = 0;
    boolean trovato = FALSE;
    while( it != NULL && trovato == FALSE ){

      if( strcmp(tipo, it -> val) == 0 )
        trovato = TRUE;
      else{

        it = it -> succ;
        i++;

      }

    }

    if( trovato == FALSE ){

      append(tipi_figli, tipo);
      append(confronti, confronto);
      res = FALSE;

    } else {

      char precedente[20];

      get(confronti, i, precedente);
      printf("[CONFRONTI]%s %s\n", precedente, confronto);
      res = strcmp(precedente, confronto) == 0 ? FALSE : TRUE;

    }
    distruggi(coda);
  }

  return res;

}

void decodifica_hub(string str){

  int count = 0, j;
  for( j = 0; str[j] != '\0'; j++ ){
    if( str[j] == '[' || str[j] == ']'){

      if( (count == 0 && str[j] == '[')  || (count == 1 && str[j] == ']') ){

        if( str[j-1] == '_')
          str[j-1] = ' ';

        if( str[j+1] == '_')
          str[j+1] = ' ';

      }
      count += str[j] == '[' ? 1 : -1;

    }
    if( count == 0 && str[j] == '_')
      str[j] = ' ';
    if ( count == 0 && str[j] == ',' )
      str[j] = ' ';
  }

}

void aggiorna_stati(string str){

  char copia[1024];
  strcpy(copia, str);

  coda_stringhe* coda = crea_coda_da_stringa(str, " ");
  char tipo[50];

  primo(coda, tipo, TRUE);
  if( strcmp(tipo, "hub") == 0 || strcmp(tipo, "timer") == 0 ){

    decodifica_hub(copia);
    coda_stringhe* figli = crea_coda_da_stringa(copia, " ");
    char stato[400];
    primo(figli, stato, TRUE);
    primo(figli, stato, TRUE);
    primo(figli, stato, TRUE);
    primo(figli, stato, TRUE);
    while(primo(figli, stato, TRUE) == TRUE )
      if( strcmp(stato, "]") != 0)
        aggiorna_stati(stato);
    distruggi(coda);

  } else{


    nodo_stringa* it = tipi_figli -> testa;

    char confronto[20];
    primo(coda, confronto, FALSE);
    primo(coda, confronto, FALSE);

    int i = 0;
    boolean trovato = FALSE;
    while( it != NULL && trovato == FALSE ){

      if( strcmp(tipo, it -> val) == 0 )
        trovato = TRUE;
      else{

        it = it -> succ;
        i++;

      }

    }

    if ( trovato == TRUE ) {

      strcpy(it -> val, confronto);

    }
    distruggi(coda);

  }

}
