#include "comunicazione/comunicazione.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>

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

boolean manda_messaggio( const int id_destinatario, const string base_path, const string messaggio ){

  char percorso[100];
  sprintf( percorso, "%s/%d", base_path, id_destinatario );
  return send_msg( percorso, messaggio );

}

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

boolean leggi_messaggio( const int id, const string base_path, string str, int lunghezza_massima ){

  char percorso[100];
  sprintf( percorso, "%s/%d",base_path, id );
  return read_msg( percorso, str, lunghezza_massima );

}
