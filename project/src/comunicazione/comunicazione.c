#include "comunicazione.h"
#include <string.h>
#include <stdio.h>

void send_msg( const char* pipe, const char* messaggio ){

    int fifo_pipe = open(pipe, O_WRONLY | O_NONBLOCK );
    write(fifo_pipe, messaggio, strlen(messaggio)+1);
    close(fifo_pipe);

}

void manda_messaggio( const int id_destinatario, const char* base_path, const char* messaggio ){

  char percorso[100];
  sprintf( percorso, "%s/%d", base_path, id_destinatario );
  send_msg( percorso, messaggio );

}

void read_msg( const char* pipe, char* str, int lunghezza_massima ){

  int fifo_pipe = open(pipe, O_RDONLY);
  read(fifo_pipe, str, lunghezza_massima);
  close(fifo_pipe);

}

void leggi_messaggio( const int id, const char* base_path, char* str, int lunghezza_massima ){

  char percorso[100];
  sprintf( percorso, "%s/%d",base_path, id );
  read_msg( percorso, str, lunghezza_massima );

}
