#ifndef PROGETTODOMOTICA_CODA_STRINGHE_H
#define PROGETTODOMOTICA_CODA_STRINGHE_H

#include "strutture_dati/tipi_base.h"

/*
* Struttura dati che rappresenta un nodo della coda
*/
typedef struct nodo_stringa{

  string valore;
  struct nodo_stringa* succ;

} nodo_stringa;

/*
* Struttura dati che rappresenta una coda di stringhe.
* n: numero di elementi nella coda.
* testa: puntatore alla testa della coda.
* coda: puntatore alla coda della coda.
*/
typedef struct coda_stringhe{

  int n;
  nodo_stringa* testa;
  nodo_stringa* coda;

} coda_stringhe;

/*
* Dichiarazione delle funzioni per operare sulle code.
*/

coda_stringhe* crea_coda();
void distruggi_coda(coda_stringhe* coda);
boolean inserisci(coda_stringhe* coda, const string valore);
boolean primo(coda_stringhe* coda, string valore, boolean distruggu);
coda_stringhe* crea_coda_da_stringa(string str, const string separatore);

#endif
