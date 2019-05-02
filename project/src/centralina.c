#include <stdio.h>
#include <stdlib.h>
#include "comunicazione/comunicazione.h"
#include "strutture_dati/tipi_componente.h"
#include "strutture_dati/coda_stringhe.h"

void gestisci_comando( coda_stringhe* separata, string comando, int dispositivi_collegati[], int* numero_collegati,
  int id, coda_stringhe* da_gestire );

int main( int argn, char** argv ){

  const int id = 0;
  int numero_collegati = 1;
  int dispositivi_collegati[1024];
  dispositivi_collegati[0] = 10;

  registro num;
  strcpy(num.nome, "num");
  num.da_calcolare = TRUE;

  printf("Centralina\n");
  printf("Digita help per una lista dei comandi disponibili\n");

  crea_pipe( id, (string) PERCORSO_BASE_DEFAULT );
  while(1){

    printf(">>");
    char str[1000];
    fgets(str, 999, stdin);
    strtok(str, "\n");
    coda_stringhe* coda = crea_coda_da_stringa(str, " ");

    char comando[20];
    primo(coda, comando, TRUE);
    printf("Comando: %s\n", comando);

    coda_stringhe* da_gestire = crea_coda();
    gestisci_comando(coda, comando, dispositivi_collegati, &numero_collegati, id, da_gestire);

    while(primo(da_gestire, str, TRUE)){
      strtok(str, "\n");
      coda = crea_coda_da_stringa(str, " ");
      primo(coda, comando, TRUE);
      gestisci_comando(coda, comando, dispositivi_collegati, &numero_collegati, id, da_gestire);
    }

  }

}

boolean prefix(const string pre, const string str){
    if(strncmp(pre, str, strlen(pre)) == 0)
      return TRUE;
    return FALSE;
}

void gestisci_comando( coda_stringhe* separata, string comando, int dispositivi_collegati[], int* numero_collegati,
  int id, coda_stringhe* da_gestire){

  if( strcmp(comando, "help") == 0 ){

    printf("----- Lista comandi ------\n");
    printf("- list\n");
    printf("- add <device>\n");
    printf("- del <id>\n");
    printf("- link <id> to <id>\n");
    printf("- switch <id> <label> <pos>\n");
    printf("- info <id>\n");
    printf("----- Fine -----\n");

  } else if( strcmp(comando, "list") == 0 ){

    int i;
    for( i = 0; i < *numero_collegati; i++ ){

      char messaggio[1024];
      int id_figlio = dispositivi_collegati[i];
      sprintf(messaggio, "STATUSGET %d", id);
      manda_messaggio(id_figlio, (string) PERCORSO_BASE_DEFAULT, messaggio);
      printf("INVIATO: %s to %d\n", messaggio, id_figlio);
      boolean flag = FALSE;
      char msg[1024];
      while(flag == FALSE){

        leggi_messaggio(id, (string) PERCORSO_BASE_DEFAULT, msg, 1023);
        if( prefix("STATUSGETRES", msg) == TRUE )
          flag = TRUE;
        else
          inserisci(da_gestire, messaggio);
      }
      printf("%s\n", msg+12);

    }

  } else if( strcmp( comando, "del") == 0 ){

    char tmp[100];
    primo(separata, tmp, TRUE);
    int id_comp = atoi(tmp);
    manda_messaggio(id_comp, (string) PERCORSO_BASE_DEFAULT, "REMOVE");

  } else if( strcmp(comando, "switch") == 0 ){

    char label[20], pos[20];
    primo(separata, label, TRUE);
    int id_dispositivo = atoi(label);
    primo(separata, label, TRUE);
    primo(separata, pos, TRUE);

    char msg[100];
    sprintf(msg, "LABELUP %s %s", label, pos);
    manda_messaggio(id_dispositivo, (string) PERCORSO_BASE_DEFAULT, msg );


  } else if( strcmp(comando, "exit") == 0 ){

    for( int i = 0; i < *numero_collegati; i++ ){

      int id_figlio = dispositivi_collegati[i];
      manda_messaggio(id_figlio, (string) PERCORSO_BASE_DEFAULT, "REMOVE");

    }
    exit(0);

  } else if( strcmp(comando, "info") == 0 ){

    char tmp[20];
    primo(separata, tmp, TRUE);
    int id_comp = atoi(tmp);
    char messaggio[30];
    sprintf(messaggio, "STATUSGET %d", id);
    manda_messaggio(id_comp, (string) PERCORSO_BASE_DEFAULT, messaggio);
    printf("INVIATO: %s to %d\n", messaggio, id_comp);
    boolean flag = FALSE;
    char msg[1025];
    while(flag == FALSE){

      leggi_messaggio(id, (string) PERCORSO_BASE_DEFAULT, msg, 1024);
      if( prefix("STATUSGETRES", msg) == TRUE )
        flag = TRUE;
      else
        inserisci(da_gestire, msg);
    }
    printf("%s\n", msg+12);


  } else {

    printf("Comando sconosciuto: %s\n", comando);

  }

}

boolean calcola_registro_intero( const registro* r, int* res ){
  return TRUE;
}
boolean calcola_registro_stringa( const registro* r, string output){
  return TRUE;
}
