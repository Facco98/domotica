#ifndef PROGETTODOMOTICA_COMUNICAZIONE_H
#define PROGETTODOMOTICA_COMUNICAZIONE_H

#include "strutture_dati/tipi_base.h"

/*
* Dichiarazione delle funzioni.
*/
boolean manda_messaggio( const int id_destinatario, const string base_path, const string messaggio );
boolean leggi_messaggio( const int id, const string base_path, string str, int lunghezza_massima );

#endif
