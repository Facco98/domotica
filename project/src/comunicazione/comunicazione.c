#include "comunicazione/comunicazione.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>

/*
* Funzione interna per mandare un messaggio a una pipe FIFO
* pipe: percorso della pipe
* messaggio: stringa contenente il messaggio da inviare
* Return: FALSE in caso di errori, TRUE altrimenti.
*/
static boolean send_msg( const string pipe, const string messaggio ){

  boolean res = FALSE;
  int fifo_pipe = open(pipe, O_WRONLY | O_NONBLOCK );
  if( fifo_pipe >= 0 ){
    write(fifo_pipe, messaggio, strlen(messaggio)+1);
    close(fifo_pipe);
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
static boolean read_msg( const string pipe, string str, int lunghezza_massima ){

  boolean res = FALSE;
  int fifo_pipe = open(pipe, O_RDONLY);
  if( fifo_pipe > 0 ){
    read(fifo_pipe, str, lunghezza_massima);
    close(fifo_pipe);
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
  if( mkfifo(percorso, 0666) >= 0 )
    res = TRUE;
  return res;

}
