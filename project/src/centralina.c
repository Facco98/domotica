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
void gestisci_comando( coda_stringhe* separata, string comando, lista_stringhe* lista_pipes);
void gestisci_list( coda_stringhe* separata, lista_stringhe* lista_pipes);
void gestisci_del( coda_stringhe* separata, lista_stringhe* lista_pipes);
void gestisci_exit( coda_stringhe* separata, lista_stringhe* lista_pipes);
void gestisci_info( coda_stringhe* separata, lista_stringhe* lista_pipes);
void gestisci_switch( coda_stringhe* separata, lista_stringhe* lista_pipes);

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

    printf("HUB id: %s\n", id);

    while(primo(coda, tmp, TRUE) == TRUE){

      if( strcmp(tmp, "\n") == 0 ){

        stampa_componente(concat);
        strcpy(concat, "");

      } else {

        strcat(concat, tmp);

      }

    }




  }

}

const int id = 0;

int main( int argn, char** argv ){


  lista_stringhe* lista_figli = crea_lista();
  append(lista_figli, "/tmp/9");

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

    gestisci_comando(coda, comando, lista_figli);

  }

}


void gestisci_comando( coda_stringhe* separata, string comando, lista_stringhe* lista_pipes){

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

    gestisci_list(separata, lista_pipes);

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

void gestisci_list(coda_stringhe* separata, lista_stringhe* lista_pipes){

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
