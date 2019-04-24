#include "strutture_dati/coda_stringhe.h"
#include <stdlib.h>
#include <string.h>

coda_stringhe* crea_coda(){

  coda_stringhe* res = (coda_stringhe*) malloc(sizeof(coda_stringhe));
  res->n = 0;
  res->testa = NULL;
  res->coda = NULL;
  return res;

}


void distruggi_coda(coda_stringhe* coda){

  if( coda != NULL){

    while( coda -> testa != NULL ){

      nodo_stringa* tmp = coda->testa;
      coda -> testa = coda -> testa -> succ;
      free(tmp -> valore);
      free(tmp);

    }
    coda -> n = 0;
    free(coda);
  }

}


boolean inserisci(coda_stringhe* coda, const string valore){

  boolean res = FALSE;
  nodo_stringa* nuovo_nodo = (nodo_stringa*) malloc(sizeof(nodo_stringa));
  nuovo_nodo -> valore = (string) malloc(sizeof(char) * strlen(valore));
  strcpy(nuovo_nodo->valore, valore);
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

    free(nuovo_nodo ->valore);
    free(nuovo_nodo);

  }
  return res;

}


boolean primo(coda_stringhe* coda, string output, boolean distruggi_se_vuota){

  boolean res = FALSE;
  if( coda != NULL && coda->testa != NULL ){

    nodo_stringa* tmp = coda->testa;
    coda->testa = coda->testa -> succ;
    strcpy(output, tmp -> valore);
    free(tmp -> valore);
    free(tmp);
    res = TRUE;

    coda -> n--;
    if( distruggi_se_vuota &&  coda -> testa == NULL )
      distruggi_coda(coda);

  }
  return res;
}


coda_stringhe* crea_coda_da_stringa(string str, const string separatore){

  coda_stringhe* coda = crea_coda();
  char* p = strtok(str, separatore);
  while( p != NULL && inserisci(coda, p) ){

      p = strtok(NULL, separatore);

  }
  return coda;

}
