#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>

#include "strutture_dati/tipi_componente.h"
#include "strutture_dati/coda_stringhe.h"
#include "comunicazione/comunicazione.h"

boolean calcola_registro_stringa( const registro* registro, string res );
boolean calcola_registro_intero( const registro* registro, int* res );
//gestisce i tre processi che costituiscono il componente (frigo)
void crea_processi_supporto(registro* registri[], int numero_registri, boolean* aperto);
//verifica sul mio id
void gestisci_ID(coda_stringhe* istruzioni);
//terminare processo/i del frigo
void termina(int x);





long apertura;

int id;

char pipe_interna[50];
char pipe_esterna[50];

pid_t figli[2];


int main (int argn, char** argv)
{
  //stato
  boolean aperto = FALSE; //indica se il frigo è aperto o chiuso

  //interruttori ??

  //termostato

  //registri (tot 4)
  //tempo_di_utilizzo
  registro tempo_utilizzo;
  strcpy(tempo_utilizzo.nome, "time");
  tempo_utilizzo.da_calcolare = FALSE;
  tempo_utilizzo.valore.integer = 0;
  tempo_utilizzo.is_intero = TRUE;

  //tempo dopo cui si richiude automaticamente
  /* registro chiusura;
  strcpy(chiusura.nome, "delay");
  tempo_utilizzo.da_calcolare = FALSE; //??
  tempo_utilizzo.valore.integer = 0; //??
  tempo_utilizzo.is_intero = TRUE; //??
  */

  //percentuale di riempimento
  /*registro riempimento;
  strcpy(riempimento.nome, "perc");
  tempo_utilizzo.da_calcolare = FALSE; //??
  tempo_utilizzo.valore.integer = 0; //??
  tempo_utilizzo.is_intero = TRUE; //??
  */

  //temperatura interna
  /*registro temperatura;
  strcpy(temperatura.nome, "temp");
  tempo_utilizzo.da_calcolare = FALSE; //??
  tempo_utilizzo.valore.integer = 0; //??
  tempo_utilizzo.is_intero = TRUE; //??
  */

  int numero_registri = 4;
  registro* registri[] = {&tempo_utilizzo/*, &chiusura, &riempimento, &temperatura*/};

  //controllo se argomenti input sono meno di due
  if( argn < 2 )
  {
    exit(130);
  }

  //recupero l'id
  id = atoi(argv[1]);

  //altri tipi di input ????

  //salvo i percorsi delle pipe per la comunicazione interna ed esterna
  sprintf(pipe_interna, "%s/%d_int", (string) PERCORSO_BASE_DEFAULT, id);
  sprintf(pipe_esterna, "%s/%d_ext", (string) PERCORSO_BASE_DEFAULT, id);

  //gestisco i processi che costituiscono il frigo
  crea_processi_supporto(registri, numero_registri, &aperto);

}


boolean calcola_registro_stringa( const registro* registro, string res )
{
  return TRUE;
}

boolean calcola_registro_intero( const registro* registro, int* res )
{

  if( registro -> da_calcolare == FALSE || registro -> is_intero == FALSE )
    return FALSE;
  if( strcmp(registro -> nome, "time") == 0 ){

    long ora = (long) time(NULL);
    *res = (registro -> valore.integer + (ora) - accensione);

  }
  return TRUE;
}

void crea_processi_supporto(registro* registri[], int numero_registri, boolean* aperto)
{

  pid_t pid = fork(); //genero un processo identico a me
  if( pid == 0 ) //se sono il figlio
  {
    mkfifo(pipe_esterna, 0666); //creo la pipe per la comunicazione con l'umano
    while(1) //continuo a trasferire messaggi da pipe_esterna a pipe_interna
    {
      char msg[200];
      read_msg(pipe_esterna, msg, 200);
      send_msg(pipe_interna, msg);
    }

  }
  else if( pid > 0 ) //se sono il padre
  {
    figli[0] = pid; //memorizzo il process-id del figlioappena generato (fork sopra)
    pid = fork(); //genero un nuovo processo identico a me
    if( pid == 0 ) //se sono il figlio
    {
      while(1) //leggo e invio messaggi da pipe con padre (= didpositivo sopra) a pipe_interna
      {
        crea_pipe(id, (string) PERCORSO_BASE_DEFAULT); //creo pipe per comunicazione con il dispositivo padre
        char msg[200];
        leggi_messaggio(id, (string) PERCORSO_BASE_DEFAULT, msg, 199);
        send_msg(pipe_interna, msg);
        read_msg(pipe_interna, msg, 199);
        if( strcmp(msg, "DONE") != 0 ) //invio sulla pipe_padre tutto ciò che non è "DONE"
        {
          manda_messaggio(id, (string) PERCORSO_BASE_DEFAULT, msg);
        }
      }

    }
    else if( pid > 0 ) //se sono il padre
    {
      figli[1] = pid; //memorizzo process-id del figlio sopra generato (seconda fork)
      signal(SIGINT, termina); //muoio se necessario
      mkfifo(pipe_interna, 0666); //genero pipe_interna
      while(1) //resto in ascolto su pipe interna e interpreto
      {
        ascolta_e_interpreta(registri, numero_registri, aperta);
      }

    }

  }

}

void ascolta_e_interpreta(registro* registri[], int numero_registri, boolean* aperto)
{
  registro* tempo_utilizzo = registri[0];
  //resto in ascolto sulla pipe interna
  char messaggio[100];
  while( read_msg(pipe_interna, messaggio, 99) == FALSE) //leggo messaggio
  {
    perror("Errore in lettura");
  }

  //elimino "\n" finale dal messaggio
  strtok(messaggio, "\n");

  //creo la coda di stringhe dal messaggio
  coda_stringhe* istruzioni = crea_coda_da_stringa(messaggio, " ");

  //recupero il nome del comando
  char nome_comando[20];
  if(!primo(separata, nome_comando, TRUE) )
  {
    exit(140);
  }

  //gestisco il comando
  if( strcmp(nome_comando, GET_STATUS) == 0 )
  {
    gestisci_STATUSGET(istruzioni, registri, numero_registri); //recuperare lo stato del frigo
  }
  else if( strcmp(nome_comando, UPDATE_LABEL) == 0 ) //aggiornare interruttori
  {
    gestisci_LABELUP(istruzioni, registri, aperto);
  }
  else if( strcmp(nome_comando, REMOVE) == 0 ) //rimuovere dispositivo //OK
  {
    char tmp[20];
    primo(istruzioni, tmp, TRUE);
    int id_ric = atoi(tmp);
    if( id_ric == id || id_ric == ID_UNIVERSALE )
    {
      termina(0);
    }

  }
  else if( strcmp(nome_comando, ID) == 0 ) //verifica se è il mio id
  {
    gestisci_ID(istruzioni); //OK
  }
  else
  {
    printf("Comando non supportato: %s\n", nome_comando); //OK
  }

}

void gestisci_ID(coda_stringhe* istruzioni)
{
  char tmp[10];
  primo(separata, tmp, TRUE);
  if( atoi(tmp) == id )
  {
    send_msg(pipe_interna, "TRUE");
  }
  else
  {
    send_msg(pipe_interna, "FALSE");
  }
}

void termina(int x)
{
  //uccido i processi figli che costituiscono il frigo
  kill(figli[0], SIGKILL);
  kill(figli[1], SIGKILL);
  close(file);

  char pipe[50];
  sprintf(pipe, "/tmp/%d", id);
  unlink(pipe);
  unlink(pipe_esterna);
  unlink(pipe_interna);
  exit(0);

}
