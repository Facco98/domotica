#ifndef PROGETTODOMOTICA_COMUNICAZIONE_H
#define PROGETTODOMOTICA_COMUNICAZIONE_H

typedef enum boolean {FALSE, TRUE} boolean;

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
