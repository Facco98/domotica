#include "strutture_dati/lista_stringhe.h"

lista_stringhe* crea_lista(){

  lista_stringhe* res = (lista_stringhe*) malloc(sizeof(lista_stringhe));
  res -> coda = res -> testa = NULL;
  res -> n = 0;
  return res;

}

boolean get(lista_stringhe* lista, int index, string output){

  boolean res = TRUE;
  if( lista == NULL || index >= lista -> n )
    res = FALSE;
  else{

    int i;
    nodo_stringa* nodo = lista -> testa;
    for( i = 0; i < index; i++ )
      nodo = nodo -> succ;
    strcpy(output, nodo -> val);
  }
  return res;

}


boolean append(lista_stringhe* lista, string val){

  boolean res = TRUE;
  nodo_stringa* nodo = (nodo_stringa*) malloc(sizeof(nodo_stringa));
  nodo -> succ = NULL;
  nodo -> val = (string) malloc(sizeof(char) * (strlen(val) + 1));
  nodo -> prec = lista -> coda;
  strcpy(nodo -> val, val);
  lista -> n ++;
  if( lista -> testa == NULL ){

      lista -> testa = lista -> coda = nodo;

  } else{

    lista -> coda -> succ = nodo;
    lista -> coda = nodo;

  }
  return res;

}

boolean prepend(lista_stringhe* lista, string val){

  boolean res = TRUE;
  nodo_stringa* nodo = (nodo_stringa*) malloc(sizeof(nodo_stringa));
  nodo -> succ = lista -> testa;
  nodo -> val = (string) malloc(sizeof(char) * (strlen(val) + 1));
  nodo -> prec = NULL;

}

void distruggi( lista_stringhe* lista ){

  if( lista != NULL ){

    nodo_stringa* nodo = lista -> testa;
    while( nodo != NULL ){

      nodo_stringa* tmp = nodo;
      nodo = nodo -> succ;
      free(tmp);

    }
    free(lista);

  }
}

void rimuovi_nodo(lista_stringhe* lista, nodo_stringa* nodo){

  if( lista -> testa == nodo ){

    lista -> testa = nodo -> succ;
  }
  else{

    nodo -> prec -> succ = nodo -> succ;

  }

}
