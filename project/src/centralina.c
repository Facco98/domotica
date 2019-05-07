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
void gestisci_comando( coda_stringhe* separata, string comando, lista_stringhe* lista_pipes, coda_stringhe* da_gestire );

const int id = 0;

int main( int argn, char** argv ){


  lista_stringhe* lista_figli = crea_lista();
  append(lista_figli, "/tmp/10");

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

    coda_stringhe* da_gestire = crea_coda();
    gestisci_comando(coda, comando, lista_figli, da_gestire);

    while(primo(da_gestire, str, TRUE)){
      strtok(str, "\n");
      coda = crea_coda_da_stringa(str, " ");
      primo(coda, comando, TRUE);
      gestisci_comando(coda, comando, lista_figli, da_gestire);
    }

  }

}


void gestisci_comando( coda_stringhe* separata, string comando, lista_stringhe* lista_pipes, coda_stringhe* da_gestire){

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

    nodo_stringa* it = lista_pipes -> testa;
    nodo_stringa* tmp = NULL;
    while(it != NULL){

      if( tmp != NULL ){
        free(tmp);
        tmp = NULL;
      }

      string pipe_figlio = it -> val;
      char messaggio[1024];
      sprintf(messaggio, "%s %d", GET_STATUS, ID_UNIVERSALE);
      send_msg(pipe_figlio, messaggio);
      boolean flag = FALSE;
      char msg[1024];
      coda_stringhe* coda = crea_coda();
      while(flag == FALSE && it != NULL){

        if( read_msg(pipe_figlio, msg, 1023) == FALSE ){
          rimuovi_nodo(lista_pipes, it);
          tmp = it;
          flag = TRUE;
        } else {
          if( prefix(GET_STATUS_RESPONSE, msg) == TRUE ){
            flag = TRUE;
            printf("%s\n", msg+12);
          } else
            inserisci(coda, msg);
        }
      }

      while(primo(coda, msg, TRUE))
        send_msg(pipe_figlio, msg);
      it = it -> succ;

    }

    if( tmp != NULL )
      free(tmp);

  } else if( strcmp( comando, "del") == 0 ){

    char tmp[100];
    primo(separata, tmp, TRUE);
    int id_comp = atoi(tmp);
    sprintf(tmp, "%s %d", REMOVE, id_comp);
    nodo_stringa* it = lista_pipes -> testa;
    nodo_stringa* l = NULL;
    while( it != NULL ){

      string pipe = it -> val;

      if( l != NULL ){
        free(l);
        l = NULL;
      }
      if( send_msg(pipe, tmp) == FALSE ){

        rimuovi_nodo(lista_pipes, it);
        l = it;

      }
      it = it -> succ;
    }

    if( l == NULL )
      free(l);

  } else if( strcmp(comando, "switch") == 0 ){

    char label[20], pos[20];
    primo(separata, label, TRUE);
    int id_dispositivo = atoi(label);
    primo(separata, label, TRUE);
    primo(separata, pos, TRUE);

    char msg[100];
    sprintf(msg, "%s %d %s %s",UPDATE_LABEL ,id_dispositivo, label, pos);

    nodo_stringa* it = lista_pipes -> testa;
    nodo_stringa* tmp  = NULL;
    while( it != NULL){

      if( tmp != NULL ){

        free(tmp);
        tmp = NULL;

      }

      string pipe = it -> val;

      if( send_msg(pipe, msg ) == FALSE ){

        rimuovi_nodo(lista_pipes, it);
        tmp = it;

      }

      it = it -> succ;

    }



  } else if( strcmp(comando, "exit") == 0 ){

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

  } else if( strcmp(comando, "info") == 0 ){

    char tmp[20];
    primo(separata, tmp, TRUE);
    int id_comp = atoi(tmp);
    char msg[200];
    nodo_stringa* it = lista_pipes -> testa;
    nodo_stringa* l = NULL;
    boolean flag = FALSE;

    while( it != NULL && flag == FALSE ){
      if( l != NULL ){
        rimuovi_nodo(lista_pipes, it);
        free(l);
        l = NULL;
      }

      string pipe = it -> val;

      sprintf(msg, "%s %d", ID, id_comp);
      if( send_msg(pipe, msg)==FALSE || read_msg(pipe, msg, 199)==FALSE ){

        rimuovi_nodo(lista_pipes, it);
        l = it;
        it = it -> succ;

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
      } else
        printf("%s\n", msg+12);

    } else{

      printf("Non collegato direttamente\n");

    }

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
