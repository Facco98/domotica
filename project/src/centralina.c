#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include "comunicazione/comunicazione.h"
#include "strutture_dati/tipi_componente.h"
#include "strutture_dati/coda_stringhe.h"
#include "strutture_dati/lista_stringhe.h"

//Funzione che gestisce i comandi da tastiera.
void gestisci_comando( coda_stringhe* separata, string comando, lista_stringhe* lista_pipes, lista_stringhe* da_creare, lista_stringhe* dispositivi_ammessi, boolean* stato);

//funzione che gestisce il comano list(elenco di dispositivi disponibili)
void gestisci_list( coda_stringhe* separata, lista_stringhe* lista_pipes, lista_stringhe* da_creare);

//funzione che gestisce il comando del (rimozione di un dispositivo)
void gestisci_del( coda_stringhe* separata, lista_stringhe* lista_pipes, lista_stringhe* da_creare);

//funzione che gestisce il comando exit (chiudere tutto)
void gestisci_exit(coda_stringhe* separata,lista_stringhe* lista_pipes, lista_stringhe* da_creare);

//funzione che gestisce il comando info (stato di un dispositivo specifico)
void gestisci_info( coda_stringhe* separata, lista_stringhe* lista_pipes);

//funzione che gestisce il comando switch (cambiare lo stato di un dispositivo)
void gestisci_switch( coda_stringhe* separata, lista_stringhe* lista_pipes);

//funzione che gestisce comando add (aggiunge un dispositivo)
void gestisci_add(coda_stringhe* separate, lista_stringhe* lista_figli, lista_stringhe* da_creare, lista_stringhe* dispositivi_ammessi);

//funzione che gestisce il comando link (collegar eun dispositivo ad un altro)
void gestisci_link(coda_stringhe* separata, lista_stringhe* lista_pipes, lista_stringhe* da_creare);

//funzione per gestire la stampa dei componenti per il comando list
void stampa_componente_list(string msg, int indent);

//funzione per gestire la stampa dei componenti pe ril comando info
void stampa_componente_info(string msg, int indent);

//funzion eper generare un nuovo figlio
void genera_figlio( string status );

//sostituisce '_' con ' ' in una stringa
void decodifica_controllo( string str );

//sostituisce ',' con ' ' in una stringa
void decodifica_figli( string tmp );

//funzione per creare un dispositivo non direttamente connesso alla centralina
void crea_dispositivo_non_connesso(string tipo, lista_stringhe* lista_pipes, lista_stringhe* da_creare);

//funzione per creare i processi che costituiscono la centralina
void crea_processi_supporto();

//controlla se due stringh ehanno uguale suffisso
boolean suffix(const char *str, const char *suffix);

//funzione per terminare la centralina
void termina( int x );

char* trim(char* s);
char* a;

const int id = 0;
int id_successivo = 1;

char pipe_interna[100], pipe_esterna[100];

pid_t processi_supporto[2];

lista_stringhe* lista_figli;

lista_stringhe* da_creare;

lista_stringhe* dispositivi_ammessi;

int main( int argn, char** argv ){

  //creo le pipe per la comunicazione con umano e all'interno della centralina
  sprintf(pipe_interna, "%s/%d_int", (string) PERCORSO_BASE_DEFAULT, id);
  sprintf(pipe_esterna, "%s/%d_ext", (string) PERCORSO_BASE_DEFAULT, id);

  crea_processi_supporto();

}

void termina( int x ){
  //elimino i processi di supporto
  kill(processi_supporto[0], SIGKILL);
  kill(processi_supporto[1], SIGKILL);

  //mando a tutti i miei figli di morire
  nodo_stringa* it = lista_figli -> testa;
  char msg[200];
  sprintf(msg, "%s %d", REMOVE, ID_UNIVERSALE);
  while(it != NULL){

    send_msg(it -> val, msg);
    it = it -> succ;

  }
  //mando a tutti i dispositivi creati (non linkati) di morire
  it = da_creare -> testa;
  while(it != NULL){

    send_msg(it -> val, msg);
    it = it -> succ;

  }

  //elimino le pipe
  unlink(pipe_esterna);
  unlink(pipe_interna);
  exit(0);

}

void crea_processi_supporto(){

  pid_t pid = fork(); //genero un processo
  if( pid == 0 ){ //se sono il figlio
    //legge da tastiera e manda nella pipe interna
    printf("Centralina\n");
    printf("Digita help per una lista dei comandi disponibili\n");
    while(1){
      char msg[1000];
      fgets(msg, 1000, stdin);
      strtok(msg, "\n");
      send_msg(pipe_interna, msg);
    }

  } else if ( pid > 0 ){ //se sono il padre

    processi_supporto[0] = pid; //salvo process-id del figlio
    pid = fork(); //genero un nuovo processo
    if( pid == 0 ){ //se sono il figlio

      mkfifo(pipe_esterna, 0666); //creo la pipe esterna
      while(1){ //continuo a spostare messaggi da pipe_esterna a pipe_interna

        char msg[1024];
        read_msg(pipe_esterna, msg, 1024);
        send_msg(pipe_interna, msg);

      }

    } else if( pid > 0 ){ //se sono il padre

      processi_supporto[1] = pid; //mi salvo il process-id del filgio

      signal(SIGINT, termina);
      signal(SIGCHLD, SIG_IGN);
      lista_figli = crea_lista(); //creo la lista dei miei figli
      da_creare = crea_lista();
      dispositivi_ammessi = crea_lista(); //creo la lista dei dispositivi che possono essere collegati

      append(dispositivi_ammessi, "bulb");
      append(dispositivi_ammessi, "hub");
      append(dispositivi_ammessi, "timer");
      append(dispositivi_ammessi, "fridge");
      append(dispositivi_ammessi, "window");


      registro num;
      strcpy(num.nome, "num");
      num.da_calcolare = TRUE;

      boolean accesa = TRUE;

      mkfifo(pipe_interna, 0666); //creo la pipe interna
      while(1){
        //leggo i messaggi dalla pipe intrena e gestisco i comandi
        char tmp[1000];
        string str;
        read_msg(pipe_interna, tmp, 999);
        str = trim(tmp);
        strtok(str, "\n");
        coda_stringhe* coda = crea_coda_da_stringa(str, " ");

        char comando[20];
        primo(coda, comando, FALSE);

        //gestisce il comando appena ricevuto
        gestisci_comando(coda, comando, lista_figli, da_creare, dispositivi_ammessi, &accesa);

      }

    }

  }

}

void gestisci_comando( coda_stringhe* separata, string comando, lista_stringhe* lista_pipes, lista_stringhe* da_creare, lista_stringhe* dispositivi_ammessi, boolean* accesa){
  //gestisce i comandi ricevuti
  if( strcmp(comando, "LABELUP") == 0 ){ //aggiornamento interruttore

    char label[200];
    //recupero il nome dell'interruttore
    primo(separata, label, FALSE);
    primo(separata, label, FALSE);
    char pos[200];
    primo(separata, pos, FALSE); //recupero la nuova posizione dell'interruttore

    if( strcmp(label, "GENERALE") == 0 ){ //se l'ibterruttore è quello GENERALE della centralina

      *accesa = strcmp(pos, "ON") == 0 ? TRUE : FALSE; //imposto l'interruttore

    }
    free(separata);

  } else if( strcmp(comando, "exit") == 0 ){ //se ricevo comando per chiudere tutto

    gestisci_exit(separata, lista_pipes, da_creare);

  } else if( *accesa == FALSE ){ //se la centralina è spenta, informo l'utente
    printf("La centralina è spenta\n");
    free(separata);

  } else if( strcmp(comando, "help") == 0 ){ //mostra quali comandi sono disponibili

    printf("----- Lista comandi ------\n");
    printf("- list\n");
    printf("- add <device>\n");
    printf("- del <id>\n");
    printf("- link <id> to <id>\n");
    printf("- switch <id> <label> <pos>\n");
    printf("- info <id>\n");
    printf("----- Fine -----\n");

  } else if( strcmp(comando, "list") == 0 ){ //chiede lista dei dispoditivi disponibili

    gestisci_list(separata, lista_pipes, da_creare);

  } else if( strcmp(comando, "add") == 0 ){ //aggiunge un dispositivo
    //controllo di avere abbastanza argomenti
    if( separata -> n >= 1 )
      gestisci_add(separata, lista_pipes, da_creare, dispositivi_ammessi);
    else
      printf("Argomento mancante\n");

  } else if( strcmp(comando, "link") == 0 ){ //collega due dispositivi
    //controllo di avere abbastanza argomenti
    if( separata -> n >= 3 )
      gestisci_link(separata, lista_pipes, da_creare);
    else
      printf("Argomento mancante\n");

  } else if( strcmp( comando, "del") == 0 ){ //rimuove un dispositivo
    //controllo di avere abbastanza argomenti
    if( separata -> n >= 1 )
      gestisci_del(separata, lista_pipes, da_creare);
    else
      printf("Argomento mancante\n");

  } else if( strcmp(comando, "switch") == 0 ){ //aggiornamento di un interruttore
    //controllo di avere abbastanza argomenti
    if( separata -> n >= 3 )
      gestisci_switch(separata, lista_pipes);
    else
      printf("Argomento mancante\n");

  } else if( strcmp(comando, "info") == 0 ){ //richiede lo stato di un particolare dispositivo
    //controllo di avere abbastanza argomenti
    if( separata ->n >= 1 )
      gestisci_info(separata, lista_pipes);
    else
      printf("Argomento mancante");

  } else { //qualsiasi altro comando non previsto

    printf("Comando sconosciuto: %s\n", comando);
    free(separata);
  }

}

boolean calcola_registro_intero( const registro* r, int* res ){
  return TRUE;
}

boolean calcola_registro_stringa( const registro* r, string output){
  return TRUE;
}

void gestisci_list(coda_stringhe* separata, lista_stringhe* lista_pipes, lista_stringhe* da_creare){

  //
  nodo_stringa* it = lista_pipes -> testa;
  while(it != NULL){

    // Chiedo lo stato a ogni mio figlio e lo stampo
    string pipe_figlio = it -> val;
    char messaggio[1024];
    sprintf(messaggio, "%s %d", GET_STATUS, ID_UNIVERSALE);
    send_msg(pipe_figlio, messaggio);
    boolean flag = FALSE;
    char msg[1024];
    while(flag == FALSE && it != NULL){

      if( read_msg(pipe_figlio, msg, 1023) == FALSE ){
        nodo_stringa* tmp = it;
        rimuovi_nodo(lista_pipes, it);
        flag = TRUE;
        it = it -> succ;
        free(tmp -> val);
        free(tmp);
      } else {
        if( prefix(GET_STATUS_RESPONSE, msg) == TRUE ){
          flag = TRUE;
          stampa_componente_list(msg+13, 0);

        }
        it = it -> succ;
      }
    }
  }

  it = da_creare -> testa;
  if( it != NULL )
    printf("Lista dei dispositivi aggiunti ma non collegati: \n");

  // Chiedo lo stato di tutti i dispositivi di cui non si è fatta la link.
  while(it != NULL){

    string pipe_figlio = it -> val;
    char messaggio[1024];
    sprintf(messaggio, "%s %d", GET_STATUS, ID_UNIVERSALE);
    send_msg(pipe_figlio, messaggio);
    boolean flag = FALSE;
    char msg[1024];
    while(flag == FALSE && it != NULL){

      if( read_msg(pipe_figlio, msg, 1023) == FALSE ){
        nodo_stringa* tmp = it;
        rimuovi_nodo(lista_pipes, it);
        flag = TRUE;
        it = it -> succ;
        free(tmp);
      } else {
        if( prefix(GET_STATUS_RESPONSE, msg) == TRUE ){
          flag = TRUE;
          stampa_componente_list(msg+13, 0);

        }
        it = it -> succ;
      }
    }
  }
}

void gestisci_del( coda_stringhe* separata, lista_stringhe* lista_pipes, lista_stringhe* da_creare){
  //gestisce la rimozione di un dispositivo

  char id_ric[20];
  if( strtol(id_ric, &a, 10) == id ){ //se si tenta di rimuovere la centralina, dà errore

    printf("Non puoi rimuovere la centralina\n");
    return;

  }
  //recupera l'id del dispositivo da rimuovere
  primo(separata, id_ric, TRUE);
  char remove_msg[200], confirm_msg[50];
  sprintf(remove_msg, "%s %s", REMOVE, id_ric);
  sprintf(confirm_msg, "%s %s", "CONFIRM", id_ric);
  nodo_stringa* it = lista_pipes -> testa;

  while( it != NULL ){

    //printf("[HUB PIPE]%s\n", it -> val);

    char res[10];
    if( send_msg( it -> val, confirm_msg ) == FALSE || read_msg(it -> val, res, 9) == FALSE ){

      nodo_stringa* l = it;
      rimuovi_nodo(lista_pipes, it);
      it = it -> succ;
      free(l -> val);
      free(l);

    } else if( strcmp(res, "TRUE") == 0 ){

      // Se il dispotivio è direttamente mio figlio lo elimino dalla lista delle pipe.
      send_msg(it -> val, remove_msg);
      nodo_stringa* l = it;
      rimuovi_nodo(lista_pipes, it);
      it = it -> succ;
      free( l -> val);
      free(l);

    } else{
      //printf("[RES-CONFIRM]%s\n", res);
      send_msg(it -> val, remove_msg);
      read_msg(it -> val, res, 9);
      it = it -> succ;
    }

  }

  // Cerco tra i dispositivi da linkare.
  it = da_creare -> testa;
  while( it != NULL ){

    char res[10];
    if( send_msg( it -> val, confirm_msg ) == FALSE || read_msg(it -> val, res, 9) == FALSE ){

      nodo_stringa* l = it;
      rimuovi_nodo(da_creare, it);
      it = it -> succ;
      free(l -> val);
      free(l);

    } else if( strcmp(res, "TRUE") == 0 ){

      send_msg(it -> val, remove_msg);
      nodo_stringa* l = it;
      rimuovi_nodo(da_creare, it);
      it = it -> succ;
      free(l -> val);
      free(l);

    } else{
      send_msg(it -> val, remove_msg);
      read_msg( it -> val, res, 9);
      it = it -> succ;
    }

  }

}

void gestisci_switch(coda_stringhe* separata, lista_stringhe* lista_pipes){
  //gestisce l'aggiornamento di un interruttore
  char label[20], pos[20];
  primo(separata, label, TRUE); //recupera il nome dell'interruttore
  int id_dispositivo = strtol(label, &a, 10);
  primo(separata, label, TRUE);
  primo(separata, pos, TRUE); //recupera la nuova posizione

  //prepara il messaggio da inviare ai figli
  char msg[100];
  sprintf(msg, "%s %d %s %s",UPDATE_LABEL ,id_dispositivo, label, pos);


  if( id_dispositivo == id ){

    send_msg(pipe_esterna, msg);
    return;

  }

  //invia ad ogni figlio il messaggio per aggiornare l'interruttore
  nodo_stringa* it = lista_pipes -> testa;

  while( it != NULL){


    string pipe = it -> val;

    char res[10];
    if( send_msg(pipe, msg ) == FALSE || read_msg(pipe, res, 9) == FALSE ){

      nodo_stringa* tmp = it;
      rimuovi_nodo(lista_pipes, it);
      it = it -> succ;
      free(tmp -> val);
      free(tmp);

    } else
      it = it -> succ;

  }

}

void gestisci_exit(coda_stringhe* separata,lista_stringhe* lista_pipes, lista_stringhe* da_creare){
  //se riceve messaggio di chiudere tutto, chiama la funzione termina
  //che manda ad ogni figlio il messaggio di morire

  termina(0);

}

void gestisci_info(coda_stringhe* separata, lista_stringhe* lista_pipes){
  //se riceve comando info
  char tmp[20];
  primo(separata, tmp, TRUE); //recupera l'id del componente di cui si richiede la info
  int id_comp = strtol(tmp, &a, 10);

  if( id_comp == 0 ){ //se l'id corrisponde a quelllo della centralina

    printf("ID non valido\n");
    return;

  }


  char msg[200];
  nodo_stringa* it = lista_pipes -> testa;
  boolean flag = FALSE;
  while( it != NULL && flag == FALSE ){

    // Cerco tra i miei figli quello che mi può portare al figlio.
    string pipe = it -> val;

    sprintf(msg, "%s %d", ID, id_comp);
    if( send_msg(pipe, msg)==FALSE || read_msg(pipe, msg, 199)==FALSE ){

      nodo_stringa* l = it;
      rimuovi_nodo(lista_pipes, it);
      it = it -> succ;
      free(l-> val);
      free(l);
      flag = TRUE;

    } else {

      if( strcmp(msg,"TRUE") == 0 )
        flag = TRUE;
      else
        it = it -> succ;
    }

  }

  //invio il messaggio al dispositivo individuato prima, che lo manda al dispositivo interessato
  if( it != NULL ){
    sprintf(msg, "%s %d", GET_STATUS, id_comp);
    string pipe = it -> val;
    if( send_msg(pipe, msg) == FALSE || read_msg(pipe, msg, 199) == FALSE ){
      rimuovi_nodo(lista_pipes, it);
      free(it -> val);
      free(it);
    } else{
      stampa_componente_info(msg+13, 0);
    }

  } else{

    printf("Non collegato direttamente\n");

  }

}

void gestisci_add(coda_stringhe* separata, lista_stringhe* lista_figli,
  lista_stringhe* da_creare, lista_stringhe* dispositivi_ammessi){
    //gestisce l'aggiunta di un nuovo dispositivo
  char tipo[20];
  primo(separata, tipo, TRUE); //recupero il tipo di dispositivo
  nodo_stringa* it = dispositivi_ammessi -> testa;
  boolean ammesso = FALSE;
  while( it != NULL && ammesso == FALSE ){ //controllo che sia uno dei dispositivi ammessi

    if( strcmp(tipo, it -> val) == 0 )
      ammesso = TRUE;
    it = it -> succ;

  }

  if( ammesso == TRUE ){ //un avolta trovato il dispositivo
    //creo il dispositivo
    crea_dispositivo_non_connesso(tipo, lista_figli, da_creare);
    printf("Aggiunto %s con id %d\n", tipo, id_successivo-1 );

  } else { //se il dispositivo non è valido

    printf("Dispositivo %s non valido\n", tipo);

  }

}

void stampa_componente_list(string msg, int indent){

  //printf("[MSG]%s\n", msg);
  coda_stringhe* coda = crea_coda_da_stringa(msg, " "); //recupero il messaggio
  char tipo[20];
  primo(coda, tipo, FALSE); //recupero il tipo
  int i = 0;
  for( i = 0; i < indent; i++ )
    printf("  ");
  printf("- ");
  if( strcmp(tipo, "hub") == 0 ){ //se è un hub
    //recupero lo stato dell'hub
    char id[20], tmp[1024], stato[20];
    primo(coda, id, FALSE);
    primo(coda, stato, FALSE);

    printf("HUB id: %s override: %s[\n", id, stato);
    primo(coda, tmp, FALSE);
    primo(coda, tmp, FALSE);
    distruggi(coda);
    decodifica_figli(tmp);
    coda = crea_coda_da_stringa(tmp, " ");
    //chiedo a tutti i miei filgi il loro stato
    while(primo(coda, tmp, FALSE) == TRUE){

      if( strcmp(tmp, "]") != 0){
        decodifica_controllo(tmp);
        stampa_componente_list(tmp, indent+1);
      }

    }
    //stampo lo stato dell'hub e dei figli
    int i = 0;
    for( i = 0; i < indent+1; i++ )
      printf("  ");
    printf("]\n");

  }
  else if(strcmp(tipo, "timer") == 0) //se è un timer
  {
    //recupero lo stato del timer
    char id[20], tmp[1024], stato[20];
    primo(coda, id, FALSE);
    primo(coda, stato, FALSE);

    printf("TIMER id: %s override: %s[\n", id, stato);

    primo(coda, tmp, FALSE); //registro begin
    primo(coda, tmp, FALSE); //registro end ingorati

    primo(coda, tmp, FALSE); //prendo parentesi quadra da ignorare
    primo(coda, tmp, FALSE); //figlio
    decodifica_figli(tmp);
    //coda = crea_coda_da_stringa(tmp, " ");
    //recupero lo stato del figlio
    if( strcmp(tmp, "]") != 0 ){
      decodifica_controllo(tmp);
      stampa_componente_list(tmp, indent+1);
    }

    //restituisco lo stato del timer e del figlio
    int i = 0;
    for( i = 0; i < indent+1; i++ )
      printf("  ");
    printf("]\n");



  }
  else { //qualsiasi altro dispositivo
    //restituisco lo stato
    char id[20], stato[20];
    primo(coda, id, FALSE);
    primo(coda, stato, FALSE);

    printf("%s id: %s stato: %s\n", tipo, id, stato);

  }

  distruggi(coda);
}

void stampa_componente_info(string msg, int indent){
  //gesisce la stampa del comando info
  //printf("[MSG]%s\n", msg);
  coda_stringhe* coda = crea_coda_da_stringa(msg, " "); //recupero il messaggio
  char tipo[20];
  primo(coda, tipo, TRUE); //recupero il tipo del dispositivo scelto
  int i = 0;
  for( i = 0; i < indent; i++ )
    printf("  ");
  printf("- ");
  if( strcmp(tipo, "bulb") == 0 ){ //se è una lampadina
    //stampo lo stato della lampadina
    char id[20], stato[20], time[20];
    primo(coda, id, TRUE);
    primo(coda, stato, TRUE);
    primo(coda, time, TRUE);

    printf("BULB id: %s stato: %s time: %s\n", id, stato, time);

  } else if( strcmp(tipo, "hub") == 0 ){ //se è un hub
    //stampo lo stato dell'hub
    char id[20], tmp[1024], stato[20];
    primo(coda, id, TRUE);
    primo(coda, stato, TRUE);
    primo(coda, tmp, TRUE);

    printf("HUB id: %s override: %s [\n", id, stato);
    //chiedo a tutti i miei figli il loro stato
    while(primo(coda, tmp, FALSE) == TRUE){
      //stampo lo stato di ogni figlio
      if( strcmp(tmp, "]") == 0 )
        break;
      decodifica_controllo(tmp);
      stampa_componente_info(tmp, indent+1);

    }

    int i = 0;
    for( i = 0; i < indent+1; i++ )
      printf("  ");
    printf("]\n");

  }
  else if( strcmp(tipo, "window") == 0 ) //se è una finestra
  {
    //stampo lo stato della finestra
    char id[20], stato[20], time[20];
    primo(coda, id, TRUE);
    primo(coda, stato, TRUE);
    primo(coda, time, TRUE);

    printf("WINDOW id: %s stato: %s time: %s\n", id, stato, time);

  }
  else if(strcmp(tipo, "fridge") == 0) //se è un frigo
  {
    //stampo lo stato del frigo
    char id[20], stato[20], time[20], delay[20], perc[20], temp[20];
    primo(coda, id, TRUE);
    primo(coda, stato, TRUE);
    primo(coda, time, TRUE);
    primo(coda, delay, TRUE);
    primo(coda, perc, TRUE);
    primo(coda, temp, TRUE);

    printf("FRIDGE id: %s stato: %s time: %s delay: %s perc: %s temp: %s\n", id, stato, time, delay, perc, temp);

  }
  else if( strcmp(tipo, "timer") == 0) //se è un timer
  {
    //stampo lo stato del timer
    char id[20], stato[20], begin[20], end[20], tmp[1024];
    primo(coda, id, TRUE);
    primo(coda, stato, TRUE); //stato = override
    primo(coda, begin, TRUE);
    primo(coda, end, TRUE);

    printf("TIMER id: %s override: %s begin: %s end: %s[\n", id, stato, begin, end);

    primo(coda, tmp, TRUE);
    primo(coda, tmp, TRUE);
    //chiedo al figlio quale sia il suo stato e lo stampo
    if( strcmp(tmp, "]") != 0 ){
      decodifica_controllo(tmp);
      stampa_componente_info(tmp, indent+1);
    }

    int i = 0;
    for( i = 0; i < indent+1; i++ )
    {
      printf("  ");
    }
    printf("]\n");


  }


}

boolean suffix(const char *str, const char *suffix){
  //controlla se due stringhe terminano con lo stesso suffisso
    if (!str || !suffix)
        return FALSE;
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix >  lenstr)
        return FALSE;
    if( strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0 )
      return TRUE;
    return FALSE;
}

void gestisci_link(coda_stringhe* separata, lista_stringhe* lista_pipes, lista_stringhe* da_creare){


  // Recupero l'id del padre e del figlio.
  char id_componente[20], id_padre[20];
  primo(separata, id_componente, TRUE);
  primo(separata, id_padre, TRUE);
  primo(separata, id_padre, TRUE);

  if( strtol(id_componente, &a, 10) == 0 || strtol(id_componente, &a, 10) == strtol(id_padre, &a, 10) ){

    printf("ID non validi\n");
    return;

  }

  // Cerco la pipe per comunicare col padre
  nodo_stringa* pipe_padre = lista_pipes -> testa;
  char msg[200];
  char status[1024];
  sprintf(msg, "%s %s", ID, id_padre);
  boolean trovato = strtol(id_padre, &a, 10) == id ? TRUE : FALSE;
  while( pipe_padre != NULL && trovato == FALSE ){

    string pipe = pipe_padre -> val;
    char res[10];
    if( send_msg(pipe, msg) == FALSE || read_msg(pipe, res, 9) == FALSE ){

      nodo_stringa* l = pipe_padre;
      rimuovi_nodo(lista_pipes, pipe_padre);
      pipe_padre = pipe_padre -> succ;
      free(l -> val);
      free(l);

    } else if( strcmp(res, "TRUE") == 0 ){

      char stato_padre[1024];
      char tmp[1024];
      sprintf(tmp, "%s %s", GET_STATUS, id_padre);
      send_msg(pipe, tmp);
      read_msg(pipe, tmp, 1023);
      strtok(tmp, " ");
      string s = strtok(NULL, " ");
      trovato = TRUE;
      if( strcmp(s, "hub") != 0 && strcmp(s, "timer") != 0 ){
        trovato = FALSE;
        pipe_padre = pipe_padre -> succ;
      }
    } else{

      pipe_padre = pipe_padre -> succ;

    }

  }

  if( trovato == FALSE ){

    printf("ID %s non valido per l'operazione di link\n", id_padre);
    return;

  }

  // Cerco il figlio tra quelli non linkati.
  nodo_stringa* pipe_figlio = da_creare -> testa;
  trovato = FALSE;
  boolean nuovo = FALSE;
  sprintf(msg, "%s %s", ID, id_componente);
  while( pipe_figlio != NULL && trovato == FALSE ){

    string pipe = pipe_figlio -> val;
    char res[10];
    if( send_msg(pipe, msg) == FALSE || read_msg(pipe, res, 9) == FALSE ){

      nodo_stringa* l = pipe_padre;
      rimuovi_nodo(da_creare, pipe_padre);
      pipe_figlio = pipe_figlio -> succ;
      free(l -> val);
      free(l);

    } else if( strcmp(res, "TRUE") == 0 ){

      rimuovi_nodo(da_creare, pipe_figlio);
      trovato = TRUE;
      nuovo = TRUE;
      char tmp[1024];
      sprintf(tmp, "%s %s", GET_STATUS, id_componente);
      send_msg(pipe, tmp);
      read_msg(pipe, tmp, 1023);
      strcat(tmp, "\0");
      strcpy(status, tmp + strlen(GET_STATUS_RESPONSE)+1);


    } else{

      pipe_figlio = pipe_figlio -> succ;

    }

  }

  if( trovato == FALSE ){

    // Se non lo ho trovato lo cerco nell'albero.
    pipe_figlio = lista_pipes -> testa;
    trovato = FALSE;
    sprintf(msg, "%s %s", ID, id_componente);
    while( pipe_figlio != NULL && trovato == FALSE ){

      string pipe = pipe_figlio -> val;
      char res[10];
      if( send_msg(pipe, msg) == FALSE || read_msg(pipe, res, 9) == FALSE ){

        nodo_stringa* l = pipe_figlio;
        rimuovi_nodo(lista_pipes, pipe_figlio);
        pipe_figlio = pipe_figlio -> succ;
        free(l -> val);
        free(l);

      } else if( strcmp(res, "TRUE") == 0 ){

        trovato = TRUE;
        char tmp[200];
        sprintf(tmp, "%s %s", GET_STATUS, id_componente);
        send_msg(pipe, tmp);
        read_msg(pipe, tmp, 199);
        strcpy(status, tmp + strlen(GET_STATUS_RESPONSE)+1);

      } else{

        pipe_figlio = pipe_figlio -> succ;

      }

    }

  }

  if( trovato == FALSE ){

    printf("ID %s non valido per l'operazione di link\n", id_componente);
    return;

  }

  boolean da_rimuovere = FALSE;
  if( nuovo == FALSE ){

    // Se è un mio figlio mi preparo per rimuovere la sua pipe dalla lista.
    sprintf(msg, "%s %s", "CONFIRM", id_componente);
    send_msg(pipe_figlio -> val, msg);
    read_msg(pipe_figlio -> val, msg, 199);
    if( strcmp(msg, "TRUE") == 0 ){

      rimuovi_nodo(lista_pipes, pipe_figlio);
      da_rimuovere = TRUE;
    }

  }
  // Gli mando di morire.
  sprintf(msg, "%s %s", REMOVE, id_componente);
  send_msg(pipe_figlio -> val, msg);
  if( da_rimuovere == FALSE && nuovo == FALSE ){
    char res[10];
    read_msg(pipe_figlio -> val, res, 9);
  }

  // Se sono il figlio.
  if( strtol(id_padre, &a, 10) == 0 ){

    pid_t pid = fork();
    if( pid == 0 ){

      // Se sono il figlio cambio l'immagine.
      genera_figlio(status);

    } else if( pid > 0 ){

      // Aggiungo la pipe del nuovo figlio.
      char tmp[50];
      sprintf(tmp, "%s/%s", (string) PERCORSO_BASE_DEFAULT, id_componente);
      append(lista_pipes, tmp);

    }

  } else {

    // Mando al nuovo padre di generare il figlio.
    sprintf(msg, "%s %s %s", "SPAWN", id_padre, status);
    send_msg(pipe_padre -> val, msg);
    if( da_rimuovere == TRUE ){
      free(pipe_figlio -> val);
      free(pipe_figlio);
    }

  }



}

void genera_figlio( string status ){
  //genera un nuovo figlio, cambiando l'immagine del processo
  strtok(status, " ");
  char tmp[40], percorso[150];
  sprintf(percorso, "./%s.out", status);


  char* params[] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};


  params[0] = (char*) malloc(sizeof(char)*40);
  strcpy(params[0], percorso);

  params[1] = strtok(NULL, " ");


  int i = 1;
  while( params[i] != NULL ){

    i++;
    params[i] = strtok(NULL, " ");

  }

  execv(params[0], params);
  perror("exec");


}

void decodifica_figli( string tmp ){
  //sostituisce le ',' con gli ' ' in una stringa
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


void decodifica_controllo(string tmp){
//sostituisce gli '_' conn gli ' ' in una stringa
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


void crea_dispositivo_non_connesso(string tipo, lista_stringhe* lista_pipes, lista_stringhe* da_creare){
  //crea un dispositivo non direttamente connesso alla centraina
  int id_dispositivo = id_successivo++;
  pid_t pid = fork(); //genero un processo
  if( pid == 0 ){

    // Se sono il figlio ne genero un altro e termino
    pid = fork();
    if( pid == 0 ){ //se sono il figlio, eseguo il processo indicato

      char stato_figlio[50];
      sprintf(stato_figlio, "%s %d", tipo, id_dispositivo);
      genera_figlio(stato_figlio); //cambio l'immagine con quella del dispositivo da creare

    } else if ( pid > 0 ){

      //Aspetto mio figlio e poi finisco.
      int stat;
      waitpid(pid, &stat, 0);
      exit(0);

    }

  } else if( pid > 0 ){ //se sono il padre, aggiungo il figlio alla lista di quelli aggiunti ma non linkati

    char percorso[50];
    sprintf(percorso, "%s/%d", (string) PERCORSO_BASE_DEFAULT, id_dispositivo);
    append(da_creare, percorso);

  }


}

char* trim(char* s) {
    char *ptr;
    if (!s)
        return NULL;   // handle NULL string
    if (!*s)
        return s;      // handle empty string
    for (ptr = s + strlen(s) - 1; (ptr >= s) && isspace(*ptr); --ptr);
    ptr[1] = '\0';
    return s;
}
