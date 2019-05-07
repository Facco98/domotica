#include "strutture_dati/lista_stringhe.h"

/*
* Crea una lista di stringhe.
* Return: un puntatore alla lista appena creata.
*/
lista_stringhe* crea_lista(){

  lista_stringhe* res = (lista_stringhe*) malloc(sizeof(lista_stringhe));
  res -> coda = res -> testa = NULL;
  res -> n = 0;
  return res;

}

/*
* Ritorna il valore ad un dato indice.
* lista: la lista da cui estrapolare il valore.
* index: indice dell'elememnto desiderato.
* output: variabile in cui verrà copiata la stringa.
* Return: FALSE in caso di errori, TRUE altrimenti.
*/
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

/*
* Funzione che inserisce un elemento in coda a una lista.
* lista: la lista su cui operare.
* val: valore da inserire in coda.
* Return: FALSE in caso di errori, TRUE altrimenti.
*/
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

/*
* Funzione che inserisce un elemento in testa a una lista.
* lista: lista su cui operare
* val: valore da inserire in testa.
*/
boolean prepend(lista_stringhe* lista, string val){

  boolean res = TRUE;
  nodo_stringa* nodo = (nodo_stringa*) malloc(sizeof(nodo_stringa));
  nodo -> succ = lista -> testa;
  nodo -> val = (string) malloc(sizeof(char) * (strlen(val) + 1));
  nodo -> prec = NULL;
  return res;

}

/*
* Funzione che dealloca una lista di stringhe.
* lista: la lista da deallocare.
*/

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

/*
* Funzione che rimuove un dato nodo dalla lista, senza deallocarne il contenuto.
* lista: la lista su cui operare.
* nodo: il nodo da rimuovere dalla lista.
*/
void rimuovi_nodo(lista_stringhe* lista, nodo_stringa* nodo){

  if( lista -> testa == nodo ){

    lista -> testa = nodo -> succ;
  }
  else{

    nodo -> prec -> succ = nodo -> succ;

  }

}
