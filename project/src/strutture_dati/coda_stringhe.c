#include "strutture_dati/coda_stringhe.h"
#include <stdlib.h>
#include <string.h>

/*
* Crea una nuova coda.
* Return: puntatore alla coda di stringhe appena creata.
*/
coda_stringhe* crea_coda(){

  coda_stringhe* res = (coda_stringhe*) malloc(sizeof(coda_stringhe));
  res->n = 0;
  res->testa = NULL;
  res->coda = NULL;
  return res;

}

/*
* Distrugge la coda deallocando anche tutti i nodi.
* coda: la coda che deve essere distrutta.
*/
void distruggi_coda(coda_stringhe* coda){

  if( coda != NULL){

    while( coda -> testa != NULL ){

      nodo_stringa* tmp = coda->testa;
      coda -> testa = coda -> testa -> succ;
      free(tmp -> val);
      free(tmp);

    }
    coda -> n = 0;
    free(coda);
  }

}

/*
* Inserisce un elemento in una coda
* coda: coda alla quale deve essere aggiunto l'elemento.
* valore: stringa che deve essere aggiunta alla coda-
* Return: FALSE in caso di errori, TRUE altrimenti.
*/
boolean inserisci(coda_stringhe* coda, const string valore){

  boolean res = FALSE;
  nodo_stringa* nuovo_nodo = (nodo_stringa*) malloc(sizeof(nodo_stringa));
  nuovo_nodo -> val = (string) malloc(sizeof(char) * ( strlen(valore)+1 ));
  strcpy(nuovo_nodo->val, valore);
  nuovo_nodo -> succ = NULL;
  if( coda != NULL ){
    if( coda -> coda == NULL ){

      coda->testa = coda->coda = nuovo_nodo;

    } else {

      coda->coda -> succ = nuovo_nodo;
      coda->coda = nuovo_nodo;
    }
    coda -> n++;
    res = TRUE;
  } else{

    free(nuovo_nodo ->val);
    free(nuovo_nodo);

  }
  return res;

}

/*
* Funzione che legge il primo elemento della lista e lo rimuove.
* coda: la coda dalla quale prelevare l'elemento
* output: dove mettere il primo elemento della coda.
* distruggi_se_vuota: se settato a TRUE e si tenta di rimuovere l'unico elemento della coda,
* la coda viene anche distrutta.
* Return: TRUE se output è stato settato correttamente, FALSE altrimenti.
*/
boolean primo(coda_stringhe* coda, string output, boolean distruggi_se_vuota){

  boolean res = FALSE;
  if( coda != NULL && coda->testa != NULL ){

    nodo_stringa* tmp = coda->testa;
    coda->testa = coda->testa -> succ;
    strcpy(output, tmp -> val);
    free(tmp -> val);
    free(tmp);
    res = TRUE;

    coda -> n--;
    if( distruggi_se_vuota &&  coda -> testa == NULL )
      distruggi_coda(coda);

  }
  return res;
}

/*
* Funzione che "splitta" una stringa e ne ritorna una coda.
* str: la stringa da splittare. Questo valore viene modificato.
* separatore: stringa per cui eseguire lo split
* Return: puntatore a una coda i cui elementi sono le sottostringhe separate dal separatore
* specificato.
*/
coda_stringhe* crea_coda_da_stringa(string str, const string separatore){

  coda_stringhe* coda = crea_coda();
  char* p = strtok(str, separatore);
  while( p != NULL && inserisci(coda, p) ){

      p = strtok(NULL, separatore);

  }
  return coda;

}
