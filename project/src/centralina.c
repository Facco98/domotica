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

/*
* Funzione che gestisce i comandi da tastiera.
*/
void gestisci_comando( coda_stringhe* separata, string comando, lista_stringhe* lista_pipes, lista_stringhe* da_creare, lista_stringhe* dispositivi_ammessi, boolean* stato);
void gestisci_list( coda_stringhe* separata, lista_stringhe* lista_pipes, lista_stringhe* da_creare);
void gestisci_del( coda_stringhe* separata, lista_stringhe* lista_pipes, lista_stringhe* da_creare);
void gestisci_exit(coda_stringhe* separata,lista_stringhe* lista_pipes, lista_stringhe* da_creare);
void gestisci_info( coda_stringhe* separata, lista_stringhe* lista_pipes);
void gestisci_switch( coda_stringhe* separata, lista_stringhe* lista_pipes);
void gestisci_add(coda_stringhe* separate, lista_stringhe* lista_figli, lista_stringhe* da_creare, lista_stringhe* dispositivi_ammessi);
void gestisci_link(coda_stringhe* separata, lista_stringhe* lista_pipes, lista_stringhe* da_creare);
void stampa_componente_list(string msg, int indent);
void stampa_componente_info(string msg, int indent);
void genera_figlio( string status );
void decodifica_controllo( string str );
void decodifica_figli( string tmp );
void crea_dispositivo_non_connesso(string tipo, lista_stringhe* lista_pipes, lista_stringhe* da_creare);
void crea_processi_supporto();
boolean suffix(const char *str, const char *suffix);
void termina( int x );



const int id = 0;
int id_successivo = 1;

char pipe_interna[100], pipe_esterna[100];

pid_t processi_supporto[2];

lista_stringhe* lista_figli;

lista_stringhe* da_creare;

lista_stringhe* dispositivi_ammessi;

int main( int argn, char** argv ){

  sprintf(pipe_interna, "%s/%d_int", (string) PERCORSO_BASE_DEFAULT, id);
  sprintf(pipe_esterna, "%s/%d_ext", (string) PERCORSO_BASE_DEFAULT, id);
  crea_processi_supporto();


}

void termina( int x ){

  kill(processi_supporto[0], SIGKILL);
  kill(processi_supporto[1], SIGKILL);

  nodo_stringa* it = lista_figli -> testa;
  char msg[200];
  sprintf(msg, "%s %d", REMOVE, ID_UNIVERSALE);
  while(it != NULL){

    send_msg(it -> val, msg);
    it = it -> succ;

  }

  it = da_creare -> testa;
  while(it != NULL){

    send_msg(it -> val, msg);
    it = it -> succ;

  }

  unlink(pipe_esterna);
  unlink(pipe_interna);
  exit(0);

}

void crea_processi_supporto(){

  pid_t pid = fork();
  if( pid == 0 ){

    printf("Centralina\n");
    printf("Digita help per una lista dei comandi disponibili\n");
    while(1){
      char msg[1000];
      fgets(msg, 1000, stdin);
      strtok(msg, "\n");
      send_msg(pipe_interna, msg);
    }

  } else if ( pid > 0 ){

    processi_supporto[0] = pid;
    pid = fork();
    if( pid == 0 ){

      mkfifo(pipe_esterna, 0666);
      while(1){

        char msg[1024];
        read_msg(pipe_esterna, msg, 1024);
        send_msg(pipe_interna, msg);

      }

    } else if( pid > 0 ){

      processi_supporto[1] = pid;

      signal(SIGINT, termina);
      signal(SIGCHLD, SIG_IGN);
      lista_figli = crea_lista();
      da_creare = crea_lista();
      dispositivi_ammessi = crea_lista();

      append(dispositivi_ammessi, "bulb");
      append(dispositivi_ammessi, "hub");
      append(dispositivi_ammessi, "timer");
      append(dispositivi_ammessi, "fridge");
      append(dispositivi_ammessi, "window");


      registro num;
      strcpy(num.nome, "num");
      num.da_calcolare = TRUE;

      boolean accesa = TRUE;

      mkfifo(pipe_interna, 0666);
      while(1){


        char str[1000];
        read_msg(pipe_interna, str, 999);
        strtok(str, "\n");
        coda_stringhe* coda = crea_coda_da_stringa(str, " ");

        char comando[20];
        primo(coda, comando, FALSE);

        gestisci_comando(coda, comando, lista_figli, da_creare, dispositivi_ammessi, &accesa);

      }

    }

  }

}

void gestisci_comando( coda_stringhe* separata, string comando, lista_stringhe* lista_pipes, lista_stringhe* da_creare, lista_stringhe* dispositivi_ammessi, boolean* accesa){

  if( strcmp(comando, "LABELUP") == 0 ){

    char label[200];
    primo(separata, label, FALSE);
    primo(separata, label, FALSE);
    char pos[200];
    primo(separata, pos, FALSE);

    if( strcmp(label, "GENERALE") == 0 ){

      *accesa = strcmp(pos, "ON") == 0 ? TRUE : FALSE;

    }
    free(separata);

  } else if( strcmp(comando, "exit") == 0 ){

    gestisci_exit(separata, lista_pipes, da_creare);

  } else if( *accesa == FALSE ){
    printf("La centralina è spenta\n");
    free(separata);

  } else if( strcmp(comando, "help") == 0 ){

    printf("----- Lista comandi ------\n");
    printf("- list\n");
    printf("- add <device>\n");
    printf("- del <id>\n");
    printf("- link <id> to <id>\n");
    printf("- switch <id> <label> <pos>\n");
    printf("- info <id>\n");
    printf("----- Fine -----\n");

  } else if( strcmp(comando, "list") == 0 ){

    gestisci_list(separata, lista_pipes, da_creare);

  } else if( strcmp(comando, "add") == 0 ){

    gestisci_add(separata, lista_pipes, da_creare, dispositivi_ammessi);

  } else if( strcmp(comando, "link") == 0 ){

    gestisci_link(separata, lista_pipes, da_creare);

  } else if( strcmp( comando, "del") == 0 ){

    gestisci_del(separata, lista_pipes, da_creare);

  } else if( strcmp(comando, "switch") == 0 ){

    gestisci_switch(separata, lista_pipes);

  } else if( strcmp(comando, "info") == 0 ){

    gestisci_info(separata, lista_pipes);

  } else {

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

  nodo_stringa* it = lista_pipes -> testa;
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

  char id_ric[20];
  primo(separata, id_ric, TRUE);
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
      free(l -> val);
      free(l);

    } else if( strcmp(res, "TRUE") == 0 ){

      send_msg(it -> val, remove_msg);
      nodo_stringa* l = it;
      rimuovi_nodo(lista_pipes, it);
      it = it -> succ;
      free( l -> val);
      free(l);

    } else{
      printf("[RES-CONFIRM]%s\n", res);
      send_msg(it -> val, remove_msg);
      read_msg(it -> val, res, 9);
      it = it -> succ;
    }

  }

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
  char label[20], pos[20];
  primo(separata, label, TRUE);
  int id_dispositivo = atoi(label);
  primo(separata, label, TRUE);
  primo(separata, pos, TRUE);

  char msg[100];
  sprintf(msg, "%s %d %s %s",UPDATE_LABEL ,id_dispositivo, label, pos);

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


  termina(0);

}

void gestisci_info(coda_stringhe* separata, lista_stringhe* lista_pipes){

  char tmp[20];
  primo(separata, tmp, TRUE);
  int id_comp = atoi(tmp);
  char msg[200];
  nodo_stringa* it = lista_pipes -> testa;
  boolean flag = FALSE;
  while( it != NULL && flag == FALSE ){

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

  char tipo[20];
  primo(separata, tipo, TRUE);
  nodo_stringa* it = dispositivi_ammessi -> testa;
  boolean ammesso = FALSE;
  while( it != NULL && ammesso == FALSE ){

    if( strcmp(tipo, it -> val) == 0 )
      ammesso = TRUE;
    it = it -> succ;

  }

  if( ammesso == TRUE ){

    crea_dispositivo_non_connesso(tipo, lista_figli, da_creare);

  } else {

    printf("Dispositivo %s non valido\n", tipo);

  }

}

void stampa_componente_list(string msg, int indent){

  printf("[MSG]%s\n", msg);
  coda_stringhe* coda = crea_coda_da_stringa(msg, " ");
  char tipo[20];
  primo(coda, tipo, FALSE);
  int i = 0;
  for( i = 0; i < indent; i++ )
    printf("  ");
  printf("- ");
  if( strcmp(tipo, "hub") == 0 ){

    char id[20], tmp[1024], stato[20];
    primo(coda, id, FALSE);
    primo(coda, stato, FALSE);

    printf("HUB id: %s override: %s[\n", id, stato);
    primo(coda, tmp, FALSE);
    primo(coda, tmp, FALSE);
    distruggi(coda);
    decodifica_figli(tmp);
    coda = crea_coda_da_stringa(tmp, " ");
    while(primo(coda, tmp, FALSE) == TRUE){

      if( strcmp(tmp, "]") != 0){
        decodifica_controllo(tmp);
        stampa_componente_list(tmp, indent+1);
      }

    }

    int i = 0;
    for( i = 0; i < indent+1; i++ )
      printf("  ");
    printf("]\n");

  } else {

    char id[20], stato[20];
    primo(coda, id, FALSE);
    primo(coda, stato, FALSE);

    printf("%s id: %s stato: %s\n", tipo, id, stato);

  }

  distruggi(coda);
}

void stampa_componente_info(string msg, int indent){


  coda_stringhe* coda = crea_coda_da_stringa(msg, " ");
  char tipo[20];
  primo(coda, tipo, TRUE);
  int i = 0;
  for( i = 0; i < indent; i++ )
    printf("  ");
  printf("- ");
  if( strcmp(tipo, "bulb") == 0 ){

    char id[20], stato[20], time[20];
    primo(coda, id, TRUE);
    primo(coda, stato, TRUE);
    primo(coda, time, TRUE);

    printf("BULB id: %s stato: %s time: %s\n", id, stato, time);

  } else if( strcmp(tipo, "hub") == 0 ){

    char id[20], tmp[1024], stato[20];
    primo(coda, id, TRUE);
    primo(coda, stato, TRUE);

    printf("HUB id: %s override: %s [\n", id, stato);

    while(primo(coda, tmp, FALSE) == TRUE){

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
  else if( strcmp(tipo, "window") == 0 ) //finestra
  {
    char id[20], stato[20], time[20];
    primo(coda, id, TRUE);
    primo(coda, stato, TRUE);
    primo(coda, time, TRUE);

    printf("WINDOW id: %s stato: %s time: %s\n", id, stato, time);

  }
  else if(strcmp(tipo, "fridge") == 0)
  {
    char id[20], stato[20], time[20], delay[20], perc[20], temp[20];
    primo(coda, id, TRUE);
    primo(coda, stato, TRUE);
    primo(coda, time, TRUE);
    primo(coda, delay, TRUE);
    primo(coda, perc, TRUE);
    primo(coda, temp, TRUE);

    printf("FRIDGE id: %s stato: %s time: %s delay: %s perc: %s temp: %s\n", id, stato, time, delay, perc, temp);

  }
  else if( strcmp(tipo, "timer") == 0)
  {
    char id[20], stato[20], begin[20], end[20], tmp[1024];
    primo(coda, id, TRUE);
    primo(coda, stato, TRUE); //stato = override
    primo(coda, begin, TRUE);
    primo(coda, end, TRUE);

    printf("TIMER id: %s override: %s begin: %s end: %s[\n", id, stato, begin, end);

    primo(coda, tmp, TRUE);

    int count = 0;
    int j;
    for( j = 0; tmp[j] != '\0'; j++ )
    {
      if( tmp[j] == '[' || tmp[j] == ']')
      {
        if( count == 0 )
        {
          if( tmp[j-1] == '_')
          {
            tmp[j-1] = ' ';
          }
          if( tmp[j+1] == '_')
          {
            tmp[j+1] = ' ';
          }
        }
        count += tmp[j] == '[' ? 1 : -1;

      }
      if( count == 0 && tmp[j] == '_' )
      {
        tmp[j] = ' ';
      }
      if( count == 0 && tmp[j] == ',' )
      {
        tmp[j] = ' ';
      }
    }
    stampa_componente_info(tmp, indent+1);


    int i = 0;
    for( i = 0; i < indent+1; i++ )
    {
      printf("  ");
    }
    printf("]\n");


  }


}

boolean suffix(const char *str, const char *suffix){
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

  nodo_stringa* pipe_padre = lista_pipes -> testa;
  char msg[200];
  char status[1024];
  sprintf(msg, "%s %s", ID, id_padre);
  boolean trovato = atoi(id_padre) == id ? TRUE : FALSE;
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

    sprintf(msg, "%s %s", "CONFIRM", id_componente);
    send_msg(pipe_figlio -> val, msg);
    read_msg(pipe_figlio -> val, msg, 199);
    if( strcmp(msg, "TRUE") == 0 ){

      rimuovi_nodo(lista_pipes, pipe_figlio);
      da_rimuovere = TRUE;
    }

  }
  sprintf(msg, "%s %s", REMOVE, id_componente);
  send_msg(pipe_figlio -> val, msg);
  if( da_rimuovere == FALSE && nuovo == FALSE ){
    char res[10];
    read_msg(pipe_figlio -> val, res, 9);
  }

  if( atoi(id_padre) == 0 ){

    pid_t pid = fork();
    if( pid == 0 ){

      // Se sono il figlio cambio l'immagine.
      genera_figlio(status);

    } else if( pid > 0 ){


      char tmp[50];
      sprintf(tmp, "%s/%s", (string) PERCORSO_BASE_DEFAULT, id_componente);
      append(lista_pipes, tmp);

    }

  } else {

    sprintf(msg, "%s %s %s", "SPAWN", id_padre, status);
    send_msg(pipe_padre -> val, msg);
    if( da_rimuovere == TRUE ){
      free(pipe_figlio -> val);
      free(pipe_figlio);
    }

  }



}

void genera_figlio( string status ){

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

  int id_dispositivo = id_successivo++;
  pid_t pid = fork();
  if( pid == 0 ){

    // Se sono il figlio ne genero un altro e termino
    pid = fork();
    if( pid == 0 ){


      char stato_figlio[50];
      sprintf(stato_figlio, "%s %d", tipo, id_dispositivo);
      genera_figlio(stato_figlio);

    } else if ( pid > 0 ){

      int stat;
      waitpid(pid, &stat, 0);
      exit(0);

    }

  } else if( pid > 0 ){

    char percorso[50];
    sprintf(percorso, "%s/%d", (string) PERCORSO_BASE_DEFAULT, id_dispositivo);
    append(da_creare, percorso);

  }


}
