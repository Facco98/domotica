#include "strutture_dati/coda_stringhe.h"
#include <stdlib.h>
#include <string.h>

/*
* Crea una nuova coda.
* Return: puntatore alla coda di stringhe appena creata.
*/
coda_stringhe* crea_coda(){

  return crea_lista();

}

/*
* Distrugge la coda deallocando anche tutti i nodi.
* coda: la coda che deve essere distrutta.
*/
void distruggi_coda(coda_stringhe* coda){

  distruggi(coda);

}

/*
* Inserisce un elemento in una coda
* coda: coda alla quale deve essere aggiunto l'elemento.
* valore: stringa che deve essere aggiunta alla coda-
* Return: FALSE in caso di errori, TRUE altrimenti.
*/
boolean inserisci(coda_stringhe* coda, const string valore){

  return append(coda, valore);

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
