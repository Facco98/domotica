#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>

#include "strutture_dati/tipi_componente.h"
#include "strutture_dati/coda_stringhe.h"
#include "comunicazione/comunicazione.h"

int id;
lista_stringhe* lista_pipes;
char pipe_interna[50];
char pipe_esterna[50];
pid_t figli[2];

boolean calcola_registro_intero( const registro* r, int* res );
boolean calcola_registro_stringa( const registro* r, string output);
void ascolta_e_interpreta();
void termina(int x);
void crea_processi_supporto();

int main( int argn, char** argv ){

  if( argn < 2 )
    exit(136);
  id = atoi(argv[1]);

  lista_pipes = crea_lista();
  append(lista_pipes, "/tmp/10");

  sprintf(pipe_interna, "%s/%d_int", (string) PERCORSO_BASE_DEFAULT, id);
  sprintf(pipe_esterna, "%s/%d_ext", (string) PERCORSO_BASE_DEFAULT, id);

  crea_processi_supporto();


}


void ascolta_e_interpreta(){

  char messaggio_in[200];
  read_msg(pipe_interna, messaggio_in, 199);
  //fgets(messaggio_in, 199, stdin);
  strtok(messaggio_in, "\n");

  coda_stringhe* separata = crea_coda_da_stringa(messaggio_in, " ");

  char comando[50];
  primo(separata, comando, TRUE);

  if( strcmp( comando, GET_STATUS ) == 0 ){

      char id_ric[50];
      primo(separata, id_ric, TRUE);
      int id_comp = atoi(id_ric);

      if( id_comp == id || id_comp == ID_UNIVERSALE ){

        char* response = (char*) malloc(sizeof(char) * 200 * lista_pipes -> n);
        sprintf(response, "%s HUB id: %d\n" ,GET_STATUS_RESPONSE, id);
        nodo_stringa* it = lista_pipes -> testa;
        while( it != NULL ){

          string pipe = it -> val;
          char msg[200];
          sprintf(msg, "%s %d", GET_STATUS, ID_UNIVERSALE );
          if( send_msg(pipe, msg) == FALSE || read_msg(pipe, msg, 199) == FALSE  ){

            nodo_stringa* tmp = it;
            rimuovi_nodo(lista_pipes, it);
            free(tmp);


          } else {

            printf("TRE\n");
            strcat(response, msg+strlen(GET_STATUS_RESPONSE)+1);

          }
          it = it -> succ;


        }
        send_msg(pipe_interna, response);
        free(response);

    }


  } else if( strcmp(comando, UPDATE_LABEL) == 0 ){

    char id_ric[50];
    primo(separata, id_ric, TRUE);
    int id_comp = atoi(id_ric);
    if( id_comp == id || id_comp == ID_UNIVERSALE ){

      char label[50];
      primo(separata, label, TRUE);
      char pos[50];
      primo(separata, pos, TRUE);

      char msg[200];
      sprintf(msg, "%s %d %s %s", UPDATE_LABEL, ID_UNIVERSALE, label, pos);

      nodo_stringa* it = lista_pipes -> testa;
      while( it != NULL ){

        string pipe = it -> val;
        if( send_msg(pipe, msg) == FALSE ){

          nodo_stringa* tmp = it;
          rimuovi_nodo(lista_pipes, it);
          it = it -> succ;
          free(tmp);

        }

        it = it -> succ;

      }

    }

  } else if( strcmp( comando, ID ) == 0 ) {

    char id_ric[20];
    primo(separata, id_ric, TRUE);
    int id_comp = atoi(id_ric);
    if( id_comp == id || id_comp == ID_UNIVERSALE )
      send_msg(pipe_interna, "TRUE");
    else
      send_msg(pipe_interna, "FALSE");

  } else if( strcmp(comando, REMOVE) == 0 ){

    char id_ric[20];
    primo(separata, id_ric, TRUE);
    int id_comp = atoi(id_ric);
    if( id_comp == id || id_comp == ID_UNIVERSALE )
      termina(0);

  } else if( strcmp(comando, "SPAWN") == 0 ){

    char id_ric[20];
    primo(separata, id_ric, TRUE);
    int id_comp = atoi(id_ric);
    if( id_comp == id || id_comp == ID_UNIVERSALE ){

      char line[1024];
      strcpy(line, "");
      char type[100];
      char tmp[100];

      primo(separata, type, TRUE);
      while( primo(separata, tmp, TRUE) ==  TRUE ){

        strcat(line, tmp);
        strcat(line, " ");

      }
      char path[200];
      sprintf(path, "./%s.out", type );
      pid_t pid = fork();

      if( pid == 0 ){

        execlp(path, path, line, NULL);

      } else if( pid > 0 ) {

        char pipe_figlio[100];
        sprintf(pipe_figlio, "%s/%s", (string) PERCORSO_BASE_DEFAULT, strtok(line, " "));
        append(lista_pipes, pipe_figlio);

      }


    }

  }

  else {

    printf("Comando non supportato\n");

  }

}

boolean calcola_registro_intero( const registro* r, int* res ){

  return TRUE;

}


boolean calcola_registro_stringa( const registro* r, string output){

  return TRUE;

}

void termina(int x){

  kill(figli[0], SIGKILL);
  kill(figli[1], SIGKILL);

  close(file);
  char path[200];
  sprintf(path, "%s/%d", (string) PERCORSO_BASE_DEFAULT, id);
  nodo_stringa* it = lista_pipes -> testa;
  while( it != NULL ){

    string pipe = it -> val;
    char msg[50];
    sprintf(msg, "%s %d", REMOVE, ID_UNIVERSALE);
    send_msg(pipe, msg);
    it = it -> succ;

  }
  unlink(path);
  unlink(pipe_esterna);
  unlink(pipe_interna);
  exit(0);

}

void crea_processi_supporto(){

  pid_t pid = fork();
  if( pid == 0 ){

    mkfifo(pipe_esterna, 0666);
    while(1){

      char msg[200];
      read_msg(pipe_esterna, msg, 200);
      send_msg(pipe_interna, msg);

    }

  } else if( pid > 0 ){

    figli[0] = pid;
    pid = fork();
    if( pid == 0 ){

      while(1){
        crea_pipe(id, (string) PERCORSO_BASE_DEFAULT);
        char msg[200];
        leggi_messaggio(id, (string) PERCORSO_BASE_DEFAULT, msg, 199);
        printf("RICEVUTO: %s\n", msg);
        send_msg(pipe_interna, msg);
        read_msg(pipe_interna, msg, 199);
        if( strcmp(msg, "DONE") != 0 )
          manda_messaggio(id, (string) PERCORSO_BASE_DEFAULT, msg);
        else
          printf("DONE\n");
      }

    } else if( pid > 0 ) {

      figli[1] = pid;
      signal(SIGINT, termina);
      mkfifo(pipe_interna, 0666);
      while(1){
        ascolta_e_interpreta();
      }

    }

  }

}
