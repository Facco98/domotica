#include "comunicazione/comunicazione.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>

char GET_STATUS[] = "STATUSGET";
char GET_STATUS_RESPONSE[] = "STATUSGETRES";
char ID[] = "ID";
char UPDATE_LABEL[] = "LABELUP";
char REMOVE[] = "REMOVE";

/*
* Il file descriptor dell'ultimo file aperto.
*/
int file = -1;

sem_t* sem;

/*
* Funzione interna per mandare un messaggio a una pipe FIFO
* pipe: percorso della pipe
* messaggio: stringa contenente il messaggio da inviare
* Return: FALSE in caso di errori, TRUE altrimenti.
*/
boolean send_msg( const string pipe, const string messaggio ){

  boolean res = FALSE;
  if( access(pipe, F_OK) == -1 )
    return FALSE;
  char percorso[100];
  file = open(pipe, O_WRONLY);
  if( file >= 0 ){
    write(file, messaggio, strlen(messaggio)+1);
    close(file);
    res = TRUE;
  }
  return res;

}

/*
* Funzione esposta per mandare un messaggio.
* id_destinatario: id del componente destinatario
* base_path: stringa contenente il percorso dove cercare la pipe
* messaggio: stringa contenente il messaggio da inviare
* Return: FALSE in caso di errori, TRUE altrimenti.
*/
boolean manda_messaggio( const int id_destinatario, const string base_path, const string messaggio ){

  char percorso[100];
  sprintf( percorso, "%s/%d", base_path, id_destinatario );
  return send_msg( percorso, messaggio );

}


/*
* Funzione interna per leggere un messaggio da una pipe FIFO
* pipe: stringa contenente il percorso della pipe FIFO
* str: dove verrà messo il messaggio letto.
* lunghezza_massima: lunghezza massima del messaggio da leggere.
* Return: FALSE in caso di errori, TRUE altrimenti.
*/
boolean read_msg( const string pipe, string str, int lunghezza_massima ){

  boolean res = FALSE;
  char percorso[100];
  sprintf(percorso, "/sem_%s", pipe);
  int file = open(pipe, O_RDONLY);
  if( file > 0 ){
    read(file, str, lunghezza_massima);
    close(file);
    res = TRUE;
  }
  return res;

}


/*
* Funzione interna per leggere un messaggio.
* id: identificativo del componente
* base_path: percorso dove verrà cercata la pipe FIFO
* str: dove verrà messo il messaggio letto.
* lunghezza_massima: lunghezza massima del messaggio da leggere.
* Return: FALSE in caso di errori, TRUE altrimenti.
*/
boolean leggi_messaggio( const int id, const string base_path, string str, int lunghezza_massima ){

  char percorso[100];
  sprintf( percorso, "%s/%d",base_path, id );
  return read_msg( percorso, str, lunghezza_massima );

}

/*
* Funzione esposta che crea la pipe
* id: identificativo del componente
* base_path: percorso dove verrà cercata la pipe FIFO
* Return: FALSE in caso di errori, TRUE altrimenti.
*/

boolean crea_pipe( const int id, const string base_path){

  char percorso[100];
  sprintf(percorso, "%s/%d", base_path, id );
  boolean res = FALSE;
  unlink(percorso);
  if( mkfifo(percorso, 0666) >= 0 )
    res = TRUE;
  sprintf(percorso, "%s/%d_int", base_path, id );
  unlink(percorso);
  if( mkfifo(percorso, 0666) >= 0 )
    res = TRUE;
  sprintf(percorso, "%s/%d_ext", base_path, id );
  unlink(percorso);
  if( mkfifo(percorso, 0666) >= 0 )
    res = TRUE;
  sprintf(percorso, "/sem_%s/%d_int", base_path, id);
  sem_unlink(percorso);
  sem = sem_open(percorso, O_CREAT, 0644, 1);
  return res;

}

void ripulisci( const int id, const string base_path ){

  char percorso[100];
  sprintf(percorso, "%s/%d", base_path, id );
  unlink(percorso);
  sprintf(percorso, "%s/%d_int", base_path, id );
  unlink(percorso);
  sprintf(percorso, "%s/%d_ext", base_path, id );
  unlink(percorso);
  sprintf(percorso, "/sem_%s/%d_int", base_path, id);
  sem_close(sem);
  sem_unlink(percorso);

}
