#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "strutture_dati/tipi_componente.h"
#include "strutture_dati/coda_stringhe.h"
#include "comunicazione/comunicazione.h"

//funzione per calcolare il valore di un registro, in particolare quello relativo al tempo di utilizzo
boolean calcola_registro_intero( registro* registro, int* res );

//funzione per gestire l'aggiornamento degli interruttori
void gestisci_LABELUP(coda_stringhe* istruzioni, rigestro* registri[], boolean* stato, boolean* apri, boolean* chiudi);

//funzione per gestire la richiesta dello stato della finestra
void gestisci_STATUSGET(coda_stringhe* istruzioni, registro* registri[], int numero_registri, int id);

//funzion eper interpretare messaggi
void ascolta_e_interpreta (int* id, registro* registri[], int numero_registri, boolean* stato,
   boolean* apri, boolean* chiudi);


long apertura; //indica quando la finestra è stata aperta (simile "accensione" della lampadina)

int main (int argn, char** argv)
{
  //stato
  boolean stato = FALSE; //indica lo stato (aperta = true, chiusa = false) della finestra

  //interruttori -> dopo essere stati "premuti" tornano subito a FALSE (= off)
  boolean apri_finestra = FALSE; //interruttore per aprire finestra
  boolean chiudi_finestra = FALSE; //interruttore per chiudere finestra

  //registro : indica il tempo di utilizzo, cioè quanto resta aperta
  registro tempo_utilizzo;
  strcpy(tempo_utilizzo.nome, "time");
  tempo_utilizzo.da_calcolare = FALSE; //?? capire bene cosa vuol dire
  tempo_utilizzo.valore.integer = 0; //?? come sopra
  tempo_utilizzo.is_intero = TRUE; //??

  //brutalmente copiata da lampadina sulla fiducia, serve?? a cosa??
  int numero_registri = 1;
  registro* registri[] = {&tempo_utilizzo};


  //gestione argomenti passati
  if(argn < 2) //troppo pochi argomenti -> do messaggio di Errore
  {
    exit(130);
  }

  //recupero l'id del componente che deve sempre essere passato per primo
  int id;
  id = atoi(argv[1]);

  //che formato possono avere gli altri comandi della lampadina??
  /*...


  ...*/

  //devo creare la PIPE ??? SI
  crea_pipe(id, (string) PERCORSO_BASE_DEFAULT);


  //la finestra non deve fare altro che restare in ascolto sulla sua pipe in attesa di comandi/messaggi
  while(1)
  {
    ascolta_e_interpreta(id, registri, numero_registri, stato, apri_finestra, chiudi_finestra); //da implementare
  }

} //qui finisce il main

boolean calcola_registro_intero( registro* registro, int* res )
{
  //se non serve calcolare il registro o se non è un registro intero, restituisco FALSE
  if( registro -> da_calcolare == FALSE || registro -> is_intero == FALSE )
    {
      return FALSE;
    }

  //se il registro si chiama "time", calcolo il tempo di utilizzo
  if( strcmp(registro -> nome, "time") == 0 )
  {
    long ora = (long) time(NULL);
    *res = (registro -> valore.integer + (ora) - accensione);
  }

  return TRUE;
}


void ascolta_e_interpreta (int* id, registro* registri[], int numero_registri, boolean* stato, boolean* apri, boolean* chiudi)
{
  registro* tempo_utilizzo = registri[0];

  //quando arriva un messaggio, leggo, tolgo "\n" alla fine, e creo la coda di stringhe dalla stringa originale
  char messaggio[100];
  while( !leggi_messaggio(*id, "/tmp", messaggio, 99)) //leggo messaggio
    perror("Errore in lettura");

  strtok(messaggio, "\n"); //elimino "\n" finale

  coda_stringhe* istruzioni = crea_coda_da_stringa(messsaggio, " "); //creo la coda di strignhe

  //inizio ad interpretare il messaggio arrivato
  char nome_comando[20];
  if(!primo(istruzioni, nome_comando, TRUE)) //se estraggo da coda vuota, messaggio di errore
  {
    exit(140);
  }

  //gestisco i vari comandi che possono arrivare
  if(strcmp(nome_comando, "LABELUP") == 0) //se il comando serve per AGGIORNARE UN INTERRUTTORE
  {
    gestisci_LABELUP(istruzioni, registri, stato, apri_finestra, chiudi_finestra);
  }
  else if(strcmp(nome_comando, "STATUSGET") == 0) //se il comando serev per restituitìre lo stato
  {
    gestisci_STATUSGET(istruzioni, registri, numero_registri, *id);
  }
  else if(strcmp(nome_comando, "REMOVE") == 0) //se il comando serve per rimuovere la finestra
  {
    //aggiugere..
    exit(0);
  }
  else //un qualsiasi altro comando non previsto
  {
    printf("Comando non supportato: %s\n", nome_comando);
  }

}


/*

  Fotmato messaggio: LABELUP ID OPEN // = aggiorna interruttore apri

  per aprire la finestra:
  ~ guardo se fienstra è già aperta
  - "schiaccio" interruttore di apertura (apri_finestra = TRUE)
  - "apro" la finestra (stato = TRUE)
  - salvo il tempo di apertura (... registro ...)
  - l'interruttore torna su off una volta premuto (apri_finestra = FALSE)


  ?
  - interruttore che torna ad off quando va eseguita come istruzione, cieè io l'ho messa alla fine
    ma potrebbe essere prima
  - controllo sullo stato per vedere se è possibile aprire la finetsra se è già aperta (?)
*/
void gestisci_LABELUP(coda_stringhe* istruzioni, rigestro* registri[], boolean* stato, boolean* apri, boolean* chiudi)
{
  registro* tempo_utilizzo = registri[0];

  char azione[20]; //azione da compiere sulla finetsra (= OPEN | CLOSE)
  primo(istruzioni, azione, TRUE); //estraggo dalla coda di stringhe l'azione da compiere

  if(strcmp(azione, "OPEN") == 0)//se devo aprire la finestra
  {
    if(*stato == TRUE)//se la finestra è GIA' APERTA
    {
      //non fa niente (invertire if e else)
    }
    else //se la finestra è chiusa
    {
      *apri = TRUE; //"schiaccio" interruttore di apertura
      *stato = TRUE; //"Apro" la finestra
      apertura = (long) time(NULL); //salvo l'ora in cui ho aperto la fienstra
      tempo_utilizzo -> da_calcolare = TRUE; // ????
      *apri = FALSE; //interruttore torna su off
    }

  }

  else if(strcmp(azione, "CLOSE") == 0) //se devo chiudere la finestra
  {
    if(*stato = FALSE) //se la finestra è GIA' CHIUSA
    {
      //stampo un qualche messaggio a video ????
    }
    else //se la finetsra è aperta
    {
      *chiudi = TRUE; //"schiaccio" l'interruttore per chiudere la finestra
      *stato = FALSE; //"chiudo" la fienstra
      //salvo il l'intervallo di tempo che è rimasta aperta
      long ora = (long) time(NULL);
      tempo_utilizzo -> valore.integer += (ora - apertura);
      tempo_utilizzo -> da_calcolare = FALSE;
    }

  }

}

//copiata brutalmente è OK o no?
void gestisci_STATUSGET(coda_stringhe* istruzioni, registro* registri[], int numero_registri, int id)
{

  char indice_ric[10];
  primo(istruzioni, indice_ric, TRUE);
  int i = 0;
  char res[1024*2];
  sprintf(res, "STATUSGETRES WINDOW, id: %d ", id );
  for( i = 0; i < numero_registri; i++ ){

    char str[1024];
    stampa_registro(registri[i], str);
    strcat(res, " ");
    strcat( res, str );

  }
  manda_messaggio(id, (string) PERCORSO_BASE_DEFAULT, res);

}
