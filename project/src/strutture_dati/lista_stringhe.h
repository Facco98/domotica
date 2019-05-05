#ifndef PROGETTODOMOTICA_LISTA_STRINGHE_H
#define PROGETTODOMOTICA_LISTA_STRINGHE_H

#include <stdlib.h>
#include <string.h>
#include "strutture_dati/tipi_base.h"


typedef struct nodo_stringa{

  string val;
  struct nodo_stringa* succ;
  struct nodo_stringa* prec;

} nodo_stringa;

/*
* Struttura dati che rappresenta una lista di stringhe.
* n: numero di elementi nella lista.
* testa: puntatore alla testa della lista.
* coda: puntatore alla coda della lista.
*/

typedef struct lista_stringhe{

  nodo_stringa* testa;
  nodo_stringa* coda;
  int n;

} lista_stringhe;

lista_stringhe* crea_lista();
boolean get(lista_stringhe* lista, int index, string output);
boolean prepend(lista_stringhe* lista, string val);
boolean append(lista_stringhe* lista, string val);
void distruggi( lista_stringhe* lista );
void rimuovi_nodo(lista_stringhe* lista, nodo_stringa* nodo);


#endif
