#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "strutture_dati/tipi_componente.h"
#include "strutture_dati/coda_stringhe.h"
#include "comunicazione/comunicazione.h"

//gestisce i processi che costituiscono il componente (timer)
void crea_processi_supporto();

//risponde TRUE se l'id è il suo
void gestisci_ID(coda_stringhe* istruzioni);

boolean calcola_registro_stringa( const registro* r, string output);

//termina il processo
void termina (int x);

//gestisce la creazione di una nuova componente
void gestisci_SPAWN(coda_stringhe* istruzioni);

//gestisce la richeista dello stato del componente collegato
void gestisci_STATUSGET(coda_stringhe* istruzioni);

//gestisce la rimozione dei componenti
void gestisci_REMOVE(coda_stringhe* istruzioni);

void ascolta_e_interpreta();





int id; //id del dispositivo

char pipe_figlio[50]; //memorizza la pipe del dispositivo a lui collegato

char pipe_interna[50]; //pipe per comunicare all'interno del timer
char pipe_esterna[50]; //pipe per comunicare con l'umano

pid_t figli[1]; //memorizza process-id del figlio


int main (int argn, char** argv)  //argomenti servono ??
{
  //stato = mirroring del dispositivo collegato

  //interruttori = mirroring del dispositivo collegato

  //registri:
  //registro che indica l'orario di attivazione --> va bene così ?
  registro attivazione;
  strcpy(attivazione.nome, "begin");
  attivazione.da_calcolare = FALSE;
  attivazione.valore.integer = 0;
  attivazione.is_intero = TRUE;

  //registro che indica l'orario di disattivazione
  registro disattivazione;
  strcpy(disattivazione.nome, "end");
  disattivazione.da_calcolare = FALSE;
  disattivazione.valore.integer = 0;
  disattivazione.is_intero = TRUE;

  int numero_registri = 2;
  registro* registri[] = {&attivazione, &disattivazione};

  if(argn < 2) //se il numero di argomenti passati a funzione è minore di 2 --> errore
  {
    exit(130);
  }

  id = atoi(argv[1]); //recupero l'id del componente

  sprintf(pipe_interna, "%s/%d_int", (string) PERCORSO_BASE_DEFAULT, id);
  sprintf(pipe_esterna, "%s/%d_ext", (string) PERCORSO_BASE_DEFAULT, id);

  crea_processi_supporto();

}

//ascolta e interpreta da mettere argomneti
void crea_processi_supporto()
{

  pid_t pid = fork(); //genera un processo identico a se stesso (timer)
  if( pid == 0 ) //se sono il figlio (= processo appena generato)
  {
    mkfifo(pipe_esterna, 0666); //creo la pipe per comunicare con l'umano
    while(1) //continua a spostare i messaggi da pipe esterna a pipe interna
    {
      char msg[200];
      read_msg(pipe_esterna, msg, 200);
      send_msg(pipe_interna, msg);
    }

  }

  else if( pid > 0 ) //se sono il padre (= processo generante)
  {
    figli[0] = pid; //salvo il process-id del processo appena generato dalla fork sopra
    pid = fork(); //genera un altro processo identico a se stesso
    if( pid == 0 ) //se sono il figlio
    {
      while(1) //sposta messaggi da pipe padre a pipe interna
      {
        crea_pipe(id, (string) PERCORSO_BASE_DEFAULT); //crea pipe_padre
        char msg[200];
        leggi_messaggio(id, (string) PERCORSO_BASE_DEFAULT, msg, 199);
        printf("RICEVUTO: %s\n", msg);
        send_msg(pipe_interna, msg);
        read_msg(pipe_interna, msg, 199);
        if( strcmp(msg, "DONE") != 0 ) //se riceve messaggi diversi da "DONE" li rinvia alla pipe_padre
        {
          manda_messaggio(id, (string) PERCORSO_BASE_DEFAULT, msg);
        }
      }

    }
    else if( pid > 0 ) //se sono il padre
    {
      figli[1] = pid; //mi salvo il process-id del figlio creato dalla precedente fork
      signal(SIGINT, termina); //se riceve segnale di morte, muore
      mkfifo(pipe_interna, 0666); //crea la pipe interna
      while(1) //resta in perenne attesa sulla sua pipe interna
      {
        ascolta_e_interpreta(); //--> da implementare
      }

    }

  }

}

void ascolta_e_interpreta()
{
  char messaggio[200];
  read_msg(pipe_interna, messaggio, 199); //leggo messaggio da pipe_interna
  strtok(messaggio, "\n"); //elimino lo "\n" alla fine della stringa

  coda_stringhe* istruzioni = crea_coda_da_stringa(messaggio, " ");


  char comando[50];
  primo(istruzioni, comando, TRUE); //recupero il nome del comando

  if( strcmp( comando, GET_STATUS ) == 0 ) //stato del componente collegato
  {
    gestisci_STATUSGET(istruzioni); //OK
  }
  else if( strcmp(comando, UPDATE_LABEL) == 0 ) //aggiornamento interruttori
  {
    gestisci_LABELUP(istruzioni);
  }
  else if( strcmp( comando, ID ) == 0 ) //risponde TRUE se l'id è il proprio, FALSE altrimenti
  {
    gestisci_ID(istruzioni); //OK
  }
  else if( strcmp(comando, REMOVE) == 0 ) //rimuovi timer o dispositivo collegato
  {
    gestisci_REMOVE(istruzioni); //OK
  }
  else if( strcmp(comando, "SPAWN") == 0 ) //genera figlio
  {
    gestisci_SPAWN(istruzioni); //OK
  }
  else //altri comandi non supportati
  {
    printf("Comando non supportato\n");
  }


}

void gestisci_STATUSGET(coda_stringhe* istruzioni)
{
  char id_ric[50];
  primo(istruzioni, id_ric, TRUE);
  int id_comp = atoi(id_ric);

  if( id_comp == id || id_comp == ID_UNIVERSALE )
  {
    // Creo il messaggio contenente la risposta.
    char response[200];
    sprintf(response, "%s TIMER id: %d\n" ,GET_STATUS_RESPONSE, id);

    char msg[200];
    sprintf(msg, "%s %d", GET_STATUS, ID_UNIVERSALE );
    if( pipe_figlio == NULL ||send_msg(pipe_figlio, msg) == FALSE || read_msg(pipe_figlio, msg, 199) == FALSE  )
    {
      pipe_figlio = NULL;
    }
    else
    {
      strcat(response, msg+strlen(GET_STATUS_RESPONSE)+1);
    }
    // Rispondo sulla pipe_interna.
    send_msg(pipe_interna, response);
    free(response);

  }
  else
  {
    send_msg(pipe_interna, "DONE");
  }


}

void gestisci_ID(coda_stringhe* istruzioni)
{
  // Recupero l'ID e rispondo se è il mio o no.
  char id_ric[20];
  primo(istruzioni, id_ric, TRUE);
  int id_comp = atoi(id_ric);
  if( id_comp == id || id_comp == ID_UNIVERSALE )
  {
    send_msg(pipe_interna, "TRUE");
  }
  else
  {
    send_msg(pipe_interna, "FALSE");
  }

}

boolean calcola_registro_stringa( const registro* r, string output)
{
  return TRUE;
}

void termina (int x)
{
  kill(figlio[0], SIGKILL); //uccido i processini
  kill(figlio[1], SIGKILL);

  close(file); //chiudo il file descriptor

  char path[200];
  sprintf(path, "%s/%d", (string) PERCORSO_BASE_DEFAULT, id);

  //mando al figlio messaggio di morte
  char msg[50];
  sprintf(msg, "%s %d", REMOVE, ID_UNIVERSALE);
  if(pipe_figlio != NULL)
  {
    send_msg(pipe_figlio, msg);
  }

  // Distruggo tutte le pipe.
  unlink(pipe_figlio); //pipe con il figlio
  unlink(pipe_esterna); //pipe con umano
  unlink(pipe_interna); //pipe interna
  exit(0); //chiudi tutto

}

void gestisci_REMOVE(coda_stringhe* istruzioni)
{
  // Recupero l'ID e in caso mi termino.
  char id_ric[20];
  primo(istruzioni, id_ric, TRUE);
  int id_comp = atoi(id_ric);
  if( id_comp == id || id_comp == ID_UNIVERSALE )
  {
    termina(0);
  }
}

void gestisci_SPAWN(coda_stringhe* istruzioni)
{
  char id_ric[20];
  primo(istruzioni, id_ric, TRUE); //recupero l'id
  int id_comp = atoi(id_ric);

  if( id_comp == id || id_comp == ID_UNIVERSALE ) //se sono io
  {

    char line[1024];
    strcpy(line, "");
    char type[100];
    char tmp[100];

    // Recupero il tipo di componente.
    primo(separata, type, TRUE);
    while( primo(separata, tmp, TRUE) ==  TRUE )
    {
      // Creo il resto dei parametri da passare su linea di comando.
      strcat(line, tmp);
      strcat(line, " ");
    }

    char path[200];
    sprintf(path, "./%s.out", type );
    pid_t pid = fork(); //genero un processo identico a me

    if( pid == 0 ) //se sono il figlio -> eseguo il componente da generare nel nuovo processo
    {
      // Se sono il figlio cambio l'immagine.
      execlp(path, path, line, NULL);
    }
    else if( pid > 0 ) //se sono il padre
    {
      // Se sono il padre aggiungo alla mia lista di pipes la pipe del figlio appena creato.
      sprintf(pipe_figlio, "%s/%s", (string) PERCORSO_BASE_DEFAULT, strtok(line, " "));
    }

  }

  send_msg(pipe_interna, "DONE");

}
