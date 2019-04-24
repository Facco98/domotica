#ifndef PROGETTODOMOTICA_TIPI_COMPONENTI_H
#define PROGETTODOMOTICA_TIPI_COMPONENTI_H

#include "strutture_dati/tipi_base.h"

/*
* Struttura dati che rappresenta un intero o una stringa.
*/
typedef union string_or_int{

  int integer;
  char str[100];

} string_or_int;


/*
* Tipo di dato che rappresenta un registor di un componente.
*/
typedef struct registro{

  char nome[50];
  string_or_int valore;
  boolean da_calcolare;

} registro;

#endif
