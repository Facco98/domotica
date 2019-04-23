#ifndef PROGETTODOMOTICA_COMUNICAZIONE_H
#define PROGETTODOMOTICA_COMUNICAZIONE_H

#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>

void manda_messaggio( const int id_destinatario, const char* base_path, const char* messaggio );
void leggi_messaggio( const int id, const char* base_path, char* str, int lunghezza_massima );

#endif
