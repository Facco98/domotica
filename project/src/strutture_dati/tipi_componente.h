#ifndef PROGETTODOMOTICA_TIPI_COMPONENTI_H
#define PROGETTODOMOTICA_TIPI_COMPONENTI_H

#include "strutture_dati/tipi_base.h"
#include <string.h>

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
  boolean is_intero;

} registro;

extern const char PERCORSO_BASE_DEFAULT[];

/*
* Dichiarazione delle funzioni di utilità per lavorare sui tipi dei componenti.
*/
int cerca_registro_da_nome( registro* registri[], const int n, const string nome);
boolean stampa_registro(const registro* r, string output);
extern boolean calcola_registro_intero( const registro* r, int* res );
extern boolean calcola_registro_stringa( const registro* r, string output);

#endif
