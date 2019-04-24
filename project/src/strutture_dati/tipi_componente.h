#ifndef PROGETTODOMOTICA_TIPI_COMPONENTI_H
#define PROGETTODOMOTICA_TIPI_COMPONENTI_H

#include "strutture_dati/tipi_base.h"

typedef union string_or_int{

  int integer;
  char str[100];

} string_or_int;

typedef struct registro{

  char nome[50];
  string_or_int valore;
  boolean da_calcolare;

} registro;

#endif
