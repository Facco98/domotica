#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "comunicazione/comunicazione.h"
#include "strutture_dati/tipi_componente.h"
#include "strutture_dati/coda_stringhe.h"
#include "strutture_dati/lista_stringhe.h"

/*
* Funzione che gestisce i comandi da tastiera.
*/
void gestisci_comando( coda_stringhe* separata, string comando, lista_stringhe* lista_pipes, lista_stringhe* da_creare, lista_stringhe* dispositivi_ammessi);
void gestisci_list( coda_stringhe* separata, lista_stringhe* lista_pipes, lista_stringhe* da_creare);
void gestisci_del( coda_stringhe* separata, lista_stringhe* lista_pipes);
void gestisci_exit( coda_stringhe* separata, lista_stringhe* lista_pipes);
void gestisci_info( coda_stringhe* separata, lista_stringhe* lista_pipes);
void gestisci_switch( coda_stringhe* separata, lista_stringhe* lista_pipes);
void gestisci_add(coda_stringhe* separate, lista_stringhe* da_creare, lista_stringhe* dispositivi_ammessi);
void stampa_componente(string msg);
boolean suffix(const char *str, const char *suffix);



const int id = 0;
int id_successivo = id + 1;

int main( int argn, char** argv ){


  lista_stringhe* lista_figli = crea_lista();
  append(lista_figli, "/tmp/9");

  lista_stringhe* da_creare = crea_lista();
  lista_stringhe* dispositivi_ammessi = crea_lista();
  append(dispositivi_ammessi, "bulb");
  append(dispositivi_ammessi, "hub");
  append(dispositivi_ammessi, "timer");
  append(dispositivi_ammessi, "fridge");
  append(dispositivi_ammessi, "window");


  registro num;
  strcpy(num.nome, "num");
  num.da_calcolare = TRUE;

  printf("Centralina\n");
  printf("Digita help per una lista dei comandi disponibili\n");

  crea_pipe(id, (string) PERCORSO_BASE_DEFAULT);
  while(1){

    printf(">>");
    char str[1000];
    fgets(str, 999, stdin);
    strtok(str, "\n");
    coda_stringhe* coda = crea_coda_da_stringa(str, " ");

    char comando[20];
    primo(coda, comando, TRUE);

    gestisci_comando(coda, comando, lista_figli, da_creare, dispositivi_ammessi);

  }

}


void gestisci_comando( coda_stringhe* separata, string comando, lista_stringhe* lista_pipes, lista_stringhe* da_creare, lista_stringhe* dispositivi_ammessi){

  if( strcmp(comando, "help") == 0 ){

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

    gestisci_add(separata, da_creare, dispositivi_ammessi);

  } else if( strcmp(comando, "link") == 0 ){

    char id_componente[20], id_padre[20];
    primo(separata, id_componente, TRUE);
    primo(separata, id_padre, TRUE);
    primo(separata, id_padre, TRUE);

    char path[100];
    nodo_stringa* nodo_elemento = NULL;
    nodo_stringa* pipe_componente = NULL;
    sprintf(path, "%s/%s", (string) PERCORSO_BASE_DEFAULT, id_padre);
    if( access(path, F_OK) == -1 ){

      printf("Errore: il dispositivo con id %s non è collegato al sistema\n", id_padre);
      return;

    }

    sprintf(path, "%s/%s", (string) PERCORSO_BASE_DEFAULT, id_componente);
    char status[200];
    strcpy(status, "");
    if( access(path, F_OK) == -1 ){

      // Lo cerco nella lista;
      nodo_stringa* it = da_creare -> testa;
      boolean trovato = FALSE;
      while( it != NULL && trovato == FALSE ){

        if( suffix(it->val, id_componente) == TRUE ){

          strcpy(status, it->val);
          trovato = TRUE;

        } else
          it = it -> succ;

      }
      if( trovato == TRUE )
        nodo_elemento = it;
      printf("[LOCALE]%s\n", status);

    } else {

      // Lo cerco nel sistema
      nodo_stringa* it = lista_pipes -> testa;
      boolean trovato = FALSE;
      char tmp[200];
      sprintf(tmp, "%s %s", ID, id_componente);
      while( it != NULL && trovato == FALSE ){

        char res[10];
        if( send_msg(it -> val, tmp) == FALSE || read_msg(it -> val, res, 9) == FALSE){

          nodo_stringa* l = it;
          rimuovi_nodo(lista_pipes, it);
          it = it -> succ;
          free(l);

        } else if( strcmp(res, "TRUE") == 0 ){

          trovato = TRUE;

        } else{

          printf("[RES]%s\n", res);
          it = it -> succ;

        }

      }

      if( trovato == FALSE ){

        printf("Il dispositivo con id %s non esiste\n", id_componente);
        return;

      }

      sprintf(tmp, "%s %s", GET_STATUS, id_componente);
      if( send_msg(it -> val, tmp) == FALSE || read_msg(it -> val, status, 199) == FALSE ){

        nodo_stringa* l = it;
        rimuovi_nodo(lista_pipes, it);
        it = it -> succ;
        free(l);


      } else {

        strcpy(status, status+strlen(GET_STATUS_RESPONSE)+1);
        pipe_componente = it;

      }


    }

    if( strcmp(status, "") == 0 ){
      printf("Il dispositivo con id %s non esiste\n", id_componente);
      return;
    }

    if( nodo_elemento == NULL ){

      printf("UNO\n");
      boolean flag = FALSE;
      char tmp[100];


      nodo_stringa* it = lista_pipes -> testa;
      sprintf(tmp, "%s %s", "CONFIRM", id_componente);
      while( it != NULL && flag == FALSE ){

        char res[10];
        if( send_msg( it -> val, tmp ) == FALSE || read_msg(it -> val, res, 9) == FALSE ){

          nodo_stringa* l = it;
          rimuovi_nodo(lista_pipes, it);
          it = it -> succ;
          free(l);

        } else if( strcmp(res, "TRUE") == 0 ){

          nodo_stringa* l = it;
          rimuovi_nodo(lista_pipes, it);
          it = it -> succ;
          free(l);
          flag = TRUE;

        } else
          it = it -> succ;

      }

      sprintf(tmp, "%s %s", REMOVE, id_componente);
      send_msg(pipe_componente -> val, tmp);
      printf("DUE\n");


    } else {

      printf("TRE\n");
      rimuovi_nodo(da_creare, nodo_elemento);
      free(nodo_elemento);
      printf("QUATTRO\n");

    }

    if( atoi(id_padre) == id ){

      pid_t pid = fork();
      if( pid == 0 ){

        char type[50];
        char line[1024];

        coda_stringhe* coda = crea_coda_da_stringa(status, " ");
        primo(coda, type, TRUE);
        char percorso[50];
        sprintf(percorso, "./%s.out", type );

        primo(coda, line, TRUE);

        while( primo(coda, type, TRUE) == TRUE ){

          strcat(line, " ");
          strcat(line, type);

        }

        execlp(percorso, percorso, line, NULL);

      } else if(pid > 0) {

        char pipe[100];
        sprintf(pipe, "%s/%s", (string) PERCORSO_BASE_DEFAULT, id_componente);
        append(lista_pipes, pipe);

      }


    } else {

      char tmp[100];
      sprintf(tmp, "%s %s", ID, id_padre);
      nodo_stringa* it = lista_pipes -> testa;
      boolean flag = FALSE;

      while( it != NULL && flag == FALSE ){

        char res[10];
        if( send_msg(it -> val, tmp) == FALSE || read_msg(it -> val, res, 9) == FALSE ){

          nodo_stringa* l = it;
          rimuovi_nodo(lista_pipes, it);
          it = it -> succ;
          free(l);


        } else if( strcmp(res, "TRUE") == 0){

          flag = TRUE;

        } else {

          it = it -> succ;

        }

      }

      if( flag == FALSE ){

        printf("Impossibile comunicare col padre\n");
        append(da_creare, status);
        return;

      }

      sprintf(tmp, "SPAWN %s %s", id_padre, status);
      send_msg(it -> val, tmp);

    }

  } else if( strcmp( comando, "del") == 0 ){

    gestisci_del(separata, lista_pipes);

  } else if( strcmp(comando, "switch") == 0 ){

    gestisci_switch(separata, lista_pipes);

  } else if( strcmp(comando, "exit") == 0 ){

    gestisci_exit(separata, lista_pipes);

  } else if( strcmp(comando, "info") == 0 ){

    gestisci_info(separata, lista_pipes);

  } else {

    printf("Comando sconosciuto: %s\n", comando);

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
        free(tmp);
      } else {
        if( prefix(GET_STATUS_RESPONSE, msg) == TRUE ){
          flag = TRUE;
          stampa_componente(msg+13);

        }
        it = it -> succ;
      }
    }
  }

  it = da_creare -> testa;
  if( it != NULL )
    printf("Lista dei dispositivi aggiunti ma non collegati: \n");
  while( it != NULL ){

    printf("- %s\n", it -> val);
    it = it -> succ;

  }
}

void gestisci_del( coda_stringhe* separata, lista_stringhe* lista_pipes){

  char tmp[100];
  primo(separata, tmp, TRUE);
  int id_comp = atoi(tmp);
  sprintf(tmp, "%s %d", REMOVE, id_comp);
  nodo_stringa* it = lista_pipes -> testa;

  while( it != NULL ){

    string pipe = it -> val;
    if( send_msg(pipe, tmp) == FALSE ){

      nodo_stringa* l = it;
      rimuovi_nodo(lista_pipes, it);
      it = it -> succ;
      free(l);

    } else{
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

    if( send_msg(pipe, msg ) == FALSE ){

      nodo_stringa* tmp = it;
      rimuovi_nodo(lista_pipes, it);
      it = it -> succ;
      free(tmp);

    } else
      it = it -> succ;

  }

}

void gestisci_exit(coda_stringhe* separata, lista_stringhe* lista_pipes){

  nodo_stringa* it = lista_pipes -> testa;
  while(it != NULL){

    char msg[200];
    sprintf(msg, "%s %d", REMOVE, ID_UNIVERSALE);
    send_msg(it -> val, msg);
    it = it -> succ;

  }

  char percorso[50];
  sprintf(percorso, "%s/%d", PERCORSO_BASE_DEFAULT, id);
  unlink(percorso);
  exit(0);

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
      free(it);
    } else{
      stampa_componente(msg+13);
    }

  } else{

    printf("Non collegato direttamente\n");

  }

}

void gestisci_add(coda_stringhe* separata, lista_stringhe* da_creare, lista_stringhe* dispositivi_ammessi){

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

    char tmp[100];
    sprintf(tmp, "%s %d", tipo, id_successivo++);
    append(da_creare, tmp);
    printf("Aggiuto %s con id %d\n", tipo, id_successivo-1 );

  } else {

    printf("Dispositivo %s non valido\n", tipo);

  }

}

void stampa_componente(string msg){

  coda_stringhe* coda = crea_coda_da_stringa(msg, " ");
  char tipo[20];
  primo(coda, tipo, TRUE);
  if( strcmp(tipo, "bulb") == 0 ){

    char id[20], time[20];
    primo(coda, id, TRUE);
    primo(coda, time, TRUE);

    printf("BULB id: %s time: %s\n", id, time);

  } else if( strcmp(tipo, "hub") == 0 ){

    char id[20], tmp[1024], concat[1024];
    strcpy(concat, "");
    primo(coda, id, TRUE);

    printf("HUB id: %s[\n", id);

    while(primo(coda, tmp, TRUE) == TRUE){

      if( strcmp(tmp, "\n") == 0 ){

        stampa_componente(concat);
        strcpy(concat, "");

      } else {

        strcat(concat, " ");
        strcat(concat, tmp);

      }

    }

    if( strcmp(concat, "") != 0 )
      stampa_componente(concat);

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
