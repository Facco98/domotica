#ifndef PROGETTODOMOTICA_COMUNICAZIONE_H
#define PROGETTODOMOTICA_COMUNICAZIONE_H

#include "strutture_dati/tipi_base.h"

extern int file;
extern char GET_STATUS[], GET_STATUS_RESPONSE[], ID[], UPDATE_LABEL[], REMOVE[];

/*
* Dichiarazione delle funzioni.
*/
boolean manda_messaggio( const int id_destinatario, const string base_path, const string messaggio );
boolean leggi_messaggio( const int id, const string base_path, string str, int lunghezza_massima );
boolean send_msg( const string pipe, const string messaggio );
boolean read_msg( const string pipe, string str, int lunghezza_massima );
boolean crea_pipe( const int id, const string base_path);

#endif
