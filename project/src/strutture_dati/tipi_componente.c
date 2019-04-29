#include "strutture_dati/tipi_componente.h"

const char PERCORSO_BASE_DEFAULT[] = "/tmp";

int cerca_registro_da_nome(registro* registri[], int n, string nome){

  int i = 0;
  for( i = 0; i < n; i++ )
    if( strcmp(registri[i] -> nome, nome) == 0 )
      return i;
  return -1;

}
