#include "strutture_dati/tipi_componente.h"

const char PERCORSO_BASE_DEFAULT[] = "/tmp";
const int ID_UNIVERSALE = 0;

int cerca_registro_da_nome( registro* registri[], const int n, const string nome){

  int i = 0;
  for( i = 0; i < n; i++ )
    if( strcmp(registri[i] -> nome, nome) == 0 )
      return i;
  return -1;

}

boolean stampa_registro(const registro* r, string output){

  if( r == NULL )
    return FALSE;
  registro reg = *r;
  if( reg.is_intero == TRUE ){
    int n = reg.valore.integer;
    if( reg.da_calcolare == TRUE ){
      calcola_registro_intero(r, &n);
    }
    sprintf(output, "%d", n);
  } else{
    strcpy(output, reg.valore.str);
    if( reg.da_calcolare == TRUE )
      calcola_registro_stringa(r, output);
    sprintf(output, "%s: %s", reg.nome, output);
  }
  return TRUE;
}
