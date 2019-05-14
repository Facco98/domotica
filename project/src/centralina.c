#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "comunicazione/comunicazione.h"
#include "strutture_dati/tipi_componente.h"
#include "strutture_dati/coda_stringhe.h"
#include "strutture_dati/lista_stringhe.h"

/*
* Funzione che gestisce i comandi da tastiera.
*/
void gestisci_comando( coda_stringhe* separata, string comando, lista_stringhe* lista_pipes, lista_stringhe* da_creare);
void gestisci_list( coda_stringhe* separata, lista_stringhe* lista_pipes, lista_stringhe* da_creare);
void gestisci_del( coda_stringhe* separata, lista_stringhe* lista_pipes);
void gestisci_exit( coda_stringhe* separata, lista_stringhe* lista_pipes);
void gestisci_info( coda_stringhe* separata, lista_stringhe* lista_pipes);
void gestisci_switch( coda_stringhe* separata, lista_stringhe* lista_pipes);
void gestisci_add( coda_stringhe* separata, lista_stringhe* lista_pipes, lista_stringhe* da_creare);
void minuscolo(string str);
boolean suffix(const char *str, const char *suffix);

void stampa_componente(string msg){

  coda_stringhe* coda = crea_coda_da_stringa(msg, " ");
  char tipo[20];
  primo(coda, tipo, TRUE);
  if( strcmp(tipo, "bulb") == 0 ){

    char id[20], time[20];
    primo(coda, id, TRUE);
    primo(coda, time, TRUE);

    printf("bulb id: %s time: %s\n", id, time);

  }

}

const int id = 0;
int id_successivo = id+1;
lista_stringhe* dispositivi_ammessi;

int main( int argn, char** argv ){


  lista_stringhe* lista_figli = crea_lista();

  lista_stringhe* da_creare = crea_lista();
  dispositivi_ammessi = crea_lista();
  append(dispositivi_ammessi, "bulb");
  append(dispositivi_ammessi, "hub");
  append(dispositivi_ammessi, "window");
  append(dispositivi_ammessi, "timer");
  append(dispositivi_ammessi, "fridge");

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

    gestisci_comando(coda, comando, lista_figli, da_creare);

  }

}


void gestisci_comando( coda_stringhe* separata, string comando, lista_stringhe* lista_pipes, lista_stringhe* da_creare){

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

    gestisci_add(separata, lista_pipes, da_creare);


  } else if( strcmp(comando, "link") == 0 ){

    char id_dispositivo[100];
    primo(separata, id_dispositivo, TRUE);
    char id_nuovo_padre[100];
    primo(separata, id_nuovo_padre, TRUE);
    primo(separata, id_nuovo_padre, TRUE);

    char tmp[200];
    sprintf(tmp, "%s/%s", (string) PERCORSO_BASE_DEFAULT, id_nuovo_padre);
    int fd = open(tmp, O_WRONLY | O_NONBLOCK);
    if( fd < 0 && atoi(id_nuovo_padre) != id )
      printf("Errore: non riesco a comunicare con il dispositivo con id %s\n", id_nuovo_padre);
    else{

      close(fd);
      sprintf(tmp, "%s/%s", (string) PERCORSO_BASE_DEFAULT, id_dispositivo);
      char status[100];
      strcpy(status, "");
      fd = open(tmp, O_WRONLY | O_NONBLOCK);
      close(fd);
      string pipe;
      nodo_stringa* elemento_lista;
      if( fd < 0 ){

        elemento_lista = da_creare -> testa;
        boolean flag = FALSE;
        while( elemento_lista != NULL && flag == FALSE ) {

          string elemento = elemento_lista -> val;
          if( suffix(elemento, id_dispositivo) == TRUE ){
            strcpy(status, elemento);
            flag = TRUE;
          }
          else
            elemento_lista = elemento_lista -> succ;

        }

      } else {

        printf("REMOTO");
        sprintf(tmp, "%s %s", ID, id_dispositivo);

        nodo_stringa* it = lista_pipes -> testa;
        boolean flag = FALSE;
        while( it != NULL && flag == FALSE ){

          pipe = it -> val;
          char res[10];
          if( send_msg(pipe, tmp) == FALSE || read_msg(pipe, res, 9) == FALSE ){

            rimuovi_nodo(lista_pipes, it);
            nodo_stringa* l = it;
            it = it -> succ;
            free(l);

          } else if( strcmp(res, "TRUE") == 0 )
            flag = TRUE;
          else
            it = it -> succ;

        }

        sprintf(tmp, "STATUSGETSIMPLE %s", id_dispositivo);
        if( send_msg(pipe, tmp) == FALSE || read_msg(pipe, status, 99) == FALSE ){

          rimuovi_nodo(lista_pipes, it);
          free(it);

        }

        if( strcmp(status, "") == 0 ){
          printf("%s di sconosciuto\n", id_dispositivo);
          return;
        }
      }

      if( fd >= 0 ){

        sprintf(tmp, "%s %s", REMOVE, id_dispositivo);
        send_msg(pipe, tmp);

      } else {

        rimuovi_nodo(da_creare, elemento_lista);
        free(elemento_lista);

      }
      if( atoi(id_nuovo_padre) == id ){

        if( fork() > 0 ){

          char pipe_path[200];
          sprintf(pipe_path, "%s/%s", (string) PERCORSO_BASE_DEFAULT, id_dispositivo);
          append(lista_pipes, pipe_path);
          nodo_stringa* iterator = lista_pipes -> testa;

        } else {

          char type[100];
          char line[1024];
          strcpy(line, "");
          strcpy(tmp, status);
          coda_stringhe* coda = crea_coda_da_stringa(tmp, " ");
          primo(coda, type, TRUE);
          while( primo(coda, tmp, TRUE) ==  TRUE ){

            // Creo il resto dei parametri da passare su linea di comando.
            strcat(line, tmp);
            strcat(line, " ");

          }
          char path[200];
          sprintf(path, "./%s.out", type );
          execlp(path, path, line, NULL);


        }

      } else {

        sprintf(tmp, "%s %s", ID, id_nuovo_padre);
        nodo_stringa* it = lista_pipes -> testa;
        boolean flag = FALSE;
        while( it != NULL && flag == FALSE ){

          string pipe = it -> val;
          char res[10];
          if( send_msg(pipe, tmp) == FALSE || read_msg(pipe, res, 9) == FALSE ){

            rimuovi_nodo(lista_pipes, it);
            nodo_stringa* l = it;
            it = it -> succ;
            free(l);


          } else if( strcmp(res, "TRUE") == 0 ){

            flag = TRUE;

          } else
            it = it -> succ;

        }

        if( flag == TRUE ){

          char msg[1024];
          sprintf(msg, "SPAWN %s", status);
          send_msg(it -> val, msg);

        } else {

          printf("Errore: non riesco a comunicare con il dispositivo con id %s", id_nuovo_padre);

        }


      }

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

  nodo_stringa* it = da_creare -> testa;
  if( it != NULL )
    printf("Dispositivi aggiunti ma non creati ( tipo id ): \n");
  while( it != NULL ){

    printf("- %s\n", it -> val );
    it = it -> succ;
  }

  it = lista_pipes -> testa;
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

void gestisci_add(coda_stringhe* separata, lista_stringhe* lista_pipes, lista_stringhe* da_creare){

  char dispositivo[200];
  primo(separata, dispositivo, TRUE);
  minuscolo(dispositivo);
  nodo_stringa* it = dispositivi_ammessi -> testa;
  boolean contenuto = FALSE;
  while( it != NULL && contenuto == FALSE ){

    if( strcmp(it -> val, dispositivo ) == 0 )
      contenuto = TRUE;

    it = it -> succ;

  }

  if( contenuto == TRUE ){
    char linea[200];
    sprintf(linea, "%s %d", dispositivo, id_successivo++);
    append(da_creare, linea);
    printf("Aggiunto: %s\n", linea);

  } else {

    printf("Dispositivo non valido: %s\n", dispositivo);

  }

}

void minuscolo(string str){

  for( int i = 0; str[i] != '\0'; i++ ){

    if( str[i] > 'A' && str[i] < 'Z' ){

      str[i] = (str[i] - 'A') + 'a';

    }

  }

}


boolean suffix(const char *str, const char *suffix)
{
    if (!str || !suffix)
        return FALSE;
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix >  lenstr)
        return FALSE;
    if( strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0)
      return TRUE;
    else
      return FALSE;
}
