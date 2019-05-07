#include "strutture_dati/tipi_base.h"

/*
* Funzione che controlla se una stringa ha un dato prefisso.
* Return TRUE se ha lo stesso prefisso, FALSE altrimenti
*/
boolean prefix(const string pre, const string str){
    if(strncmp(pre, str, strlen(pre)) == 0)
      return TRUE;
    return FALSE;
}
