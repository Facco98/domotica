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

//funzione pe rgenerare un figlio
void genera_figlio(coda_stringhe* separata);

//funzione ch ecalcola se c'è stato override manuale
boolean calcola_override(string str, lista_stringhe* tipi_figli, lista_stringhe* confronti);

//fnzione che aggiorna gli stati attesi dei figli
void aggiorna_stati(string str);

/*
* Funzione per terminare il processo quando arriva un SIGINT.
*/
void termina(int x);

//funzione che sostituisce gli '_' con gli ' ' in una stringa
void decodifica_hub(string str);

//funzion eche sostituisce ',' con ' '
void decodifica_figli( string tmp );

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

  /*
  * Inizializzo la lista delle PIPE dei miei figli.
  */

  lista_pipes = crea_lista();
  append(lista_pipes, "/tmp/10");

  tipi_figli = crea_lista();
  stati_attesi = crea_lista();

  /*
  * Leggo gli stati degli eventuali figli che devo creare.
  */
  int i;
  for( i = 4; i < argn-1; i++ ){

    decodifica_figli(argv[i]);

    coda_stringhe* coda = crea_coda();
    char *l = strtok(argv[i], " ");
    while(l!=NULL){

      inserisci(coda, l);
      l = strtok(NULL, " ");

    }

    int j = 0;
    char tmp[1024];
    while( coda -> n > 0 && primo(coda, tmp, FALSE) == TRUE  ){

      char s[1024];
      strcpy(s, tmp);
      decodifica_hub(s);

      coda_stringhe* c = NULL;
      do{

        c = crea_coda();

      } while( c == coda );
      char* appoggio = strtok(s, " ");
      while( appoggio != NULL ){

        inserisci(c, appoggio);
        appoggio = strtok(NULL, " ");

      }
      genera_figlio(c);
      distruggi(c);

    }
    distruggi(coda);


  }


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

  //printf("[HUB]%s\n", messaggio_in);
  // Divido la stringa per gli spazi.
  coda_stringhe* separata = crea_coda_da_stringa(messaggio_in, " ");

  //Recupero il comando e lo gestisco
  char comando[50];
  primo(separata, comando, FALSE);

  if( strcmp( comando, GET_STATUS ) == 0 ){ //richiesta dello STATO

    gestisci_STATUSGET(separata);

  } else if( strcmp(comando, UPDATE_LABEL) == 0 ){ //AGGIORNAMENTO degli interruttori

    gestisci_LABELUP(separata);

  } else if( strcmp( comando, ID ) == 0 ) { //id raggiungibile attraverso l'hub

    gestisci_ID(separata);

  } else if( strcmp(comando, REMOVE) == 0 ){ //rimuovi il dispositivo

    gestisci_REMOVE(separata);

  } else if( strcmp(comando, "SPAWN") == 0 ){ //aggiungi un figlio

    gestisci_SPAWN(separata);

  }  else if( strcmp(comando, "CONFIRM") == 0 ){ //id corrisponde al mio

    char tmp[20];
    primo(separata, tmp, FALSE);
    int d = atoi(tmp);
    if( d == id || d == ID_UNIVERSALE )
      send_msg(pipe_interna, "TRUE");
    else
      send_msg(pipe_interna, "FALSE");

  } else {

    send_msg(pipe_interna, "DONE");

  }
  distruggi(separata);

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
  ripulisci(id, (string) PERCORSO_BASE_DEFAULT);
  exit(0);

}

void crea_processi_supporto(){

  crea_pipe(id, (string) PERCORSO_BASE_DEFAULT);
  pid_t pid = fork(); //genero un processo identico a me
  if( pid == 0 ){ //se sono il  figlio

    // Se sono il figlio sto perennemente in ascolto sulla pipe con l'umano
    // e scrivo tutto su quella interna
    while(1){
      char msg[200];
      read_msg(pipe_esterna, msg, 200);
      sem_wait(sem);
      send_msg(pipe_interna, msg);
      read_msg(pipe_interna, msg, 199);
      sem_post(sem);
    }

  } else if( pid > 0 ){ //se sono il padre

    figli[0] = pid; //memorizzo il process-id del figlio appena generato
    pid = fork(); //genero un processo
    if( pid == 0 ){ //se sono il filgio

      // Se sono il figlio leggo dalla pipe con il controllore e invio su quella
      // interna, aspettando la risposta.
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

    } else if( pid > 0 ) { //se sono il padre

      // Se sono il padre sto in ascolto sulla pipe interna e interpreto i messaggi.
      figli[1] = pid; //salvo il process-id del figlio
      signal(SIGINT, termina);
      while(1){
        ascolta_e_interpreta(); //gestisco i messaggi in arrivo
      }

    }

  }

}

void gestisci_STATUSGET(coda_stringhe* separata){

  char id_ric[50];
  primo(separata, id_ric, FALSE);
  int id_comp = atoi(id_ric);

  //se l'id è il mio
  if( id_comp == id || id_comp == ID_UNIVERSALE ){

    // Creo il messaggio contenente la risposta.
    char* response = (char*) malloc(sizeof(char) * 200 * lista_pipes -> n);
    string my_status = (char*) malloc(sizeof(char) * 200 * lista_pipes -> n);
    sprintf(my_status, "%s hub %d" ,GET_STATUS_RESPONSE, id);
    strcpy(response, "");


    // Mando a tutti i miei figli che voglio lo stato.
    nodo_stringa* it = lista_pipes -> testa;

    boolean primo = TRUE;
    boolean override = FALSE;
    while( it != NULL ){

      string pipe = it -> val;
      char msg[200];
      sprintf(msg, "%s %d", GET_STATUS, ID_UNIVERSALE );
      if( send_msg(pipe, msg) == FALSE || read_msg(pipe, msg, 199) == FALSE  ){

        nodo_stringa* tmp = it;
        rimuovi_nodo(lista_pipes, it);
        it = it -> succ;
        free(tmp -> val);
        free(tmp);



      } else {

        if ( primo == FALSE ){
          strcat(response, ",");
        }
        primo = FALSE;
        int i = 0;
        char copia[1024];
        strcpy(copia, msg+strlen(GET_STATUS_RESPONSE)+1);
        if( override == FALSE )
          override = calcola_override(copia, tipi_figli, stati_attesi );
        for( i = 0; msg[i] != '\0'; i++ )
          if( msg[i] == ' ')
            msg[i] = '_';
        strcat(response, msg+strlen(GET_STATUS_RESPONSE)+1);
        it = it -> succ;

      }



    }
    // Rispondo sulla pipe_interna, con il mio stato e quello dei figli
    char tmp[20];
    sprintf(tmp, " %s [ ", override == TRUE ? "TRUE":"FALSE" );
    strcat(my_status, tmp);
    strcat(my_status, response);
    strcat(my_status, " ]");
    send_msg(pipe_interna, my_status);
    free(response);

  } else { //se l'id non è il mio, rimndo il messaggio a tutti i miei figli

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
        free(it -> val);
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
  primo(separata, id_ric, FALSE);
  int id_comp = atoi(id_ric);
  char msg[200];

  boolean ret = FALSE;

  //se l'id è mio
  if( id_comp == id || id_comp == ID_UNIVERSALE ){

    // Prendo il nome dell'interruttore e la nuova posizione.
    char label[50];
    primo(separata, label, FALSE);
    char pos[50];
    primo(separata, pos, FALSE);

    // Invio il messaggio a tutti i miei figli.
    sprintf(msg, "%s %d %s %s", UPDATE_LABEL, ID_UNIVERSALE, label, pos);



  } else { //se l'id non è mio rinvio il messaggio a tutti i miei figli

    sprintf(msg, "%s %s", UPDATE_LABEL, id_ric);
    char tmp[20];
    while( separata -> testa != NULL ){
      nodo_stringa* it = separata -> testa;
      separata -> testa = separata -> testa -> succ;
      strcpy(tmp, it -> val);
      free(it -> val);
      free(it);
      strcat(msg, " ");
      strcat(msg, tmp);
    }
    //free(separata);

  }

  //chiedo lo stato ai  miei figli per aggiornare la lista degli stati attesi
  nodo_stringa* it = lista_pipes -> testa;
  while( it != NULL ){

    string pipe = it -> val;
    char res[10];
    strcpy(res, "");
    if( send_msg(pipe, msg) == FALSE || read_msg(pipe, res, 9) == FALSE ){

      nodo_stringa* tmp = it;
      rimuovi_nodo(lista_pipes, it);
      it = it -> succ;
      free(tmp -> val);
      free(tmp);

    } else if( ( id_comp == id || id_comp == ID_UNIVERSALE ) && strcmp(res, "TRUE") == 0 ){


      char tmp[200];
      sprintf(tmp, "%s %d", GET_STATUS, ID_UNIVERSALE);
      send_msg(pipe, tmp);

      char stato[1024];
      read_msg(pipe, stato, 1023);
      aggiorna_stati(stato+strlen(GET_STATUS_RESPONSE)+1);
      ret = TRUE;

      it = it -> succ;

    } else
      it = it -> succ;

  }
  if( ret == TRUE )
    send_msg(pipe_interna, "TRUE");
  else
    send_msg(pipe_interna, "FALSE");

}

void gestisci_ID(coda_stringhe* separata){

  // Recupero l'ID e rispondo se è il mio o no.
  char id_ric[20];
  primo(separata, id_ric, FALSE);
  int id_comp = atoi(id_ric);
  if( id_comp == id || id_comp == ID_UNIVERSALE ) //se id è mio
    send_msg(pipe_interna, "TRUE");
  else{ //se id non è mio, invio ai miei figli il messaggio

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
        free(tmp -> val);
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
  if( id_comp == id || id_comp == ID_UNIVERSALE ) //se id è mio, termino
    termina(0);
  else{ //se id non è mio, invio messaggio di REMOVE a tutti i miei figli


    char remove_msg[200], confirm_msg[50];
    sprintf(remove_msg, "%s %s", REMOVE, id_ric);
    sprintf(confirm_msg, "%s %s", "CONFIRM", id_ric);
    nodo_stringa* it = lista_pipes -> testa;

    while( it != NULL ){

      char res[10];
      if( send_msg( it -> val, confirm_msg ) == FALSE || read_msg(it -> val, res, 9) == FALSE ){

        nodo_stringa* l = it;
        rimuovi_nodo(lista_pipes, it);
        it = it -> succ;
        free(l -> val);
        free(l);

      } else if( strcmp(res, "TRUE") == 0 ){

        send_msg(it -> val, remove_msg);
        nodo_stringa* l = it;
        rimuovi_nodo(lista_pipes, it);
        it = it -> succ;
        free(l -> val);
        free(l);

      } else{
        send_msg(it -> val, remove_msg);
        read_msg(it -> val, res, 9);
        it = it -> succ;
      }

    }
    send_msg(pipe_interna, "TRUE");

  }

}

void gestisci_SPAWN(coda_stringhe* separata){
  //controllo l'id, se è il mio genero un figlio, altrimenti invio il messaggio ai miei figli

  char id_ric[20];
  primo(separata, id_ric, FALSE);
  int id_comp = atoi(id_ric);
  if( id_comp == id || id_comp == ID_UNIVERSALE ){

    genera_figlio(separata);
    //distruggi(separata);

  } else {
    //ricostruire il messaggio e rinviarlo sotto
    char msg[1024];
    sprintf(msg, "%s %s", "SPAWN", id_ric);
    char tmp[200];
    while( separata -> testa != NULL )
    {
      nodo_stringa* it = separata -> testa;
      strcpy(tmp, it -> val);
      separata -> testa = separata -> testa -> succ;
      free(it -> val);
      free(it);
      strcat(msg, " ");
      strcat(msg, tmp);
    }
    //free(separata);

    nodo_stringa* it = lista_pipes -> testa;
    while( it != NULL ){

      string pipe_figlio = it -> val;
      if ( send_msg(pipe_figlio, msg) == FALSE ){

        nodo_stringa* l = it;
        rimuovi_nodo(lista_pipes, it);
        it = it -> succ;
        free(l -> val);
        free(l);

      } else {

        it = it -> succ;

      }

    }


  }


  send_msg(pipe_interna, "DONE");

}

void genera_figlio(coda_stringhe* separata){
  //genera un nuovo figlio
  char tmp[40], percorso[50];
  primo(separata, tmp, FALSE);
  sprintf(percorso, "./%s.out", tmp);



  pid_t pid = fork(); //genero un processo identico a me

  if( pid == 0 ){ //se sono il figlio, cambio l'immagine del processo

    char* params[] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
    // Se sono il figlio cambio l'immagine.

    params[0] = (char*) malloc(sizeof(char)*40);
    strcpy(params[0], percorso);

    primo(separata, tmp, FALSE);

    params[1] = (char*) malloc(sizeof(char)*40);
    strcpy(params[1], tmp);

    int i = 2;
    while( primo(separata, tmp, FALSE) ==  TRUE ){

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

  }

}

boolean calcola_override(string str, lista_stringhe* tipi_figli, lista_stringhe* confronti){
  //calcola se c'è stato override manuale
  boolean res = TRUE;
  char copia[1024];
  strcpy(copia, str);
  coda_stringhe* coda = crea_coda_da_stringa(str, " ");

  char tipo[20];
  primo(coda, tipo, FALSE);

  //se è un timer o un hub deve chiedere anche ai suoi figli
  if( strcmp(tipo, "hub") == 0 || strcmp(tipo, "timer") == 0 ){

    char stato[1024];
    primo(coda, stato, FALSE); // tipo
    primo(coda, stato, FALSE); // id
    primo(coda, stato, FALSE); // stato
    primo(coda, stato, FALSE); // [ BEGIN
    if( strcmp(tipo, "timer") == 0 ){

      primo(coda, stato, FALSE);
      primo(coda, stato, FALSE); //figlio del timer.

    }
    decodifica_figli(stato);
    coda_stringhe* figli = crea_coda_da_stringa(stato, " ");
    while( figli -> testa != NULL && res == FALSE ){
      nodo_stringa* it = figli -> testa;
      strcpy(stato, it -> val);
      figli -> testa = figli -> testa -> succ;
      free(it);
      if( strcmp(stato, "]") != 0 ){
        decodifica_hub(stato);
        res = calcola_override(stato, tipi_figli, confronti);
      }
    }
    distruggi(figli);

  } else { //altri tipi di dispositivi

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

      res = strcmp(precedente, confronto) == 0 ? FALSE : TRUE;

    }
    distruggi(coda);
  }

  return res;

}

void decodifica_figli( string tmp ){
  //in una strigna, sostituisce ',' con ' '
  int count = 0;
  int j;
  for( j = 0; tmp[j] != '\0'; j++ ){
    if( tmp[j] == '[' || tmp[j] == ']'){
      count += tmp[j] == '[' ? 1 : -1;
    }
    if( count == 0 && tmp[j] == ',')
      tmp[j] = ' ';
  }

}


void decodifica_hub(string tmp){
  //in una stringa, sostituisce '_' con ' '
  int count = 0, j;
  for( j = 0; tmp[j] != '\0'; j++ ){
    if( tmp[j] == '[' || tmp[j] == ']'){

      if( tmp[j] == ']')
        count -= 1;

      if( count == 0 ){

        if( tmp[j-1] == '_')
          tmp[j-1] = ' ';

        if( tmp[j+1] == '_')
          tmp[j+1] = ' ';

      }
      if( tmp[j] == '[')
      count += 1;
    }
    if( count == 0 && tmp[j] == '_')
      tmp[j] = ' ';
  }

}

void aggiorna_stati(string str){
  //aggiorna la lista degli stati attesi dei figli
  char copia[1024];
  strcpy(copia, str);

  coda_stringhe* coda = crea_coda_da_stringa(str, " ");
  char tipo[50];

  primo(coda, tipo, FALSE);
  //se è un hub o un timer deve chiedere anche ai suoi figli
  if( strcmp(tipo, "hub") == 0 || strcmp(tipo, "timer") == 0 ){

    decodifica_hub(copia);
    coda_stringhe* figli = crea_coda_da_stringa(copia, " ");
    char stato[400];
    primo(figli, stato, FALSE);
    primo(figli, stato, FALSE);
    primo(figli, stato, FALSE);
    primo(figli, stato, FALSE);
    while(primo(figli, stato, FALSE) == TRUE )
      if( strcmp(stato, "]") != 0)
        aggiorna_stati(stato);
    distruggi(coda);

  } else{ //altri dispositivi


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
