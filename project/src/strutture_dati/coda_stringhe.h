#ifndef PROGETTODOMOTICA_CODA_STRINGHE_H
#define PROGETTODOMOTICA_CODA_STRINGHE_H

#include "strutture_dati/tipi_base.h"

typedef struct nodo_stringa{

  string valore;
  struct nodo_stringa* succ;

} nodo_stringa;

typedef struct coda_stringhe{

  int n;
  nodo_stringa* testa;
  nodo_stringa* coda;

} coda_stringhe;

coda_stringhe* crea_coda();
void distruggi_coda(coda_stringhe* coda);
boolean inserisci(coda_stringhe* coda, const string valore);
boolean primo(coda_stringhe* coda, string valore, boolean distruggu);
coda_stringhe* crea_coda_da_stringa(string str, const string separatore);

#endif
