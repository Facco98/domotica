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
void crea_processi_supporto(registro* registri[], int numero_registri);

//risponde TRUE se l'id è il suo
void gestisci_CONFIRM(coda_stringhe* istruzioni);

boolean calcola_registro_stringa( const registro* r, string output);

//termina il processo
void termina (int x);

//gestisce la creazione di una nuova componente
void gestisci_SPAWN(coda_stringhe* istruzioni);

//gestisce la richeista dello stato del componente collegato
void gestisci_STATUSGET(coda_stringhe* istruzioni);

//gestisce la rimozione dei componenti
void gestisci_REMOVE(coda_stringhe* istruzioni);

void ascolta_e_interpreta(registro* registri[], int numero_registri);

void gestisci_LABELUP(coda_stringhe* istruzioni);

void genera_figlio(coda_stringhe* status);

void gestisci_ID(coda_stringhe* istruzioni);



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

  //se ricevo più argomenti da riga di Comando
  // input: id datidelfiglio
  //datidelfiglio ==> stringa separata da underscore
  if(argn >= 3)
  {
    //controllo se non ho già un figlio ?? è inutile ??
    char dati_figlio[200]; //strigna per contenere i dati di creazione del figlio con _
    strcpy(dati_figlio, argv[2]); //recupero i dati del figlio con _

    coda_stringhe* status = crea_coda_da_stringa(dati_figlio, "_"); //separo i dati del figlio dagli _ e creo una coda

    genera_figlio(status); //genero il figlio

  }

  sprintf(pipe_interna, "%s/%d_int", (string) PERCORSO_BASE_DEFAULT, id);
  sprintf(pipe_esterna, "%s/%d_ext", (string) PERCORSO_BASE_DEFAULT, id);

  crea_processi_supporto(registri, numero_registri);

}

//ascolta e interpreta da mettere argomneti
void crea_processi_supporto(registro* registri[], int numero_registri)
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
        ascolta_e_interpreta(registri, numero_registri); //--> da implementare
      }

    }

  }

}

void ascolta_e_interpreta(registro* registri[], int numero_registri)
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
    gestisci_LABELUP(istruzioni, registri, numero_registri); // !!!!!
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
  else if( strcmp(comando, "CONFIRM") == 0)
  {
    gestisci_CONFIRM(istruzioni); //OK
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
    sprintf(response, "%s timer %d" ,GET_STATUS_RESPONSE, id);

    char msg[200];
    sprintf(msg, "%s %d", GET_STATUS, ID_UNIVERSALE );
    if( strcmp(pipe_figlio, "") == 0 || send_msg(pipe_figlio, msg) == FALSE || read_msg(pipe_figlio, msg, 199) == FALSE  )
    {
      strcpy(pipe_figlio, "");
    }
    else
    {

      strcat(response, " ");
      int i = 0;
      for( i = 0; msg[i] != '\0'; i++ )
      {
        if( msg[i] == ' ')
        {
          msg[i] = '_';
        }
      }

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

void gestisci_CONFIRM(coda_stringhe* istruzioni)
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
  if(strcmp(pipe_figlio, "") != 0)
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
  else      //chiedere id al figlio, se è il figlio a dover morire
  {
    if(strcmp(pipe_figlio, "") != 0) //se il figlio esiste
    {
      //chiedo a mio figlio se è lui che deve morire
      char msg[20];
      char res[20];
      sprintf(msg, "%s %s", "CONFIRM", id_ric);

      send_msg(pipe_figlio, msg); //invio richiesta di id al dati_figlio
      read_msg(pipe_figlio, res); //leggo risposta dal dati_figlio

      if(strcmp(res, "TRUE") == 0) //se mio figlio deve morire
      {
        //elimino mio figlio
        strcpy(pipe_figlio, "");
      }

      //non decidi tu se morire muori e basta
      char die_msg[100];
      sprintf(die_msg, "%s %s", REMOVE, id_ric); //preparo il messaggio di morte per il figlio
      send_msg(pipe_figlio, die_msg);

    }
    send_msg(pipe_interna, "DONE");//informo i processini che ho concluso la missione
  }
}

//controllo se sono io a dover generare un figlio se si genero
void gestisci_SPAWN(coda_stringhe* istruzioni)
{
  char id_ric[20];
  primo(istruzioni, id_ric, TRUE);
  int id_comp = atoi(id_ric);
  if( id_comp == id || id_comp == ID_UNIVERSALE )
  {
    genera_figlio(istruzioni);
  }

}

//prendo in input una coda di strighe con i dati del filgio
void genera_figlio(coda_stringhe* status)
{
  char tmp[40], percorso[50];
  primo(status, tmp, TRUE);
  sprintf(percorso, "./%s.out", tmp);

  pid_t pid = fork();

  if( pid == 0 )
  {
    char* params[] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
    // Se sono il figlio cambio l'immagine.

    params[0] = (char*) malloc(sizeof(char)*40);
    strcpy(params[0], percorso);

    primo(status, tmp, TRUE);

    params[1] = (char*) malloc(sizeof(char)*40);
    strcpy(params[1], tmp);

    int i = 2;
    while( primo(status, tmp, TRUE) ==  TRUE )
    {
      params[i] = (char*) malloc((strlen(tmp)+1) * sizeof(char));
      strcpy(params[i], tmp);
      i++;
    }

    execv(params[0], params);

    int j = 0;
    for( j = 0; j <= i; j++ )
      free(params[j]);

  }
  else if( pid > 0 )
  {
    // Se sono il padre aggiungo alla mia lista di pipes la pipe del figlio appena creato.
    char id[30];
    primo(status, tmp, FALSE);
    char pipe_figlio[100];
    sprintf(pipe_figlio, "%s/%s", (string) PERCORSO_BASE_DEFAULT, tmp);
    distruggi(status);

  }

  send_msg(pipe_interna, "DONE");

}

//aggiornare i miei registri e di conseguenza quelli del figlio
//mi arriva un messaggio per settare i miei registri e poi io ne invio uno al figlio per settare i suoi
/*
LABELUP id interruttore(begin/end) nuovo_stato(=valore int)

*/
void gestisci_LABELUP(coda_stringhe* istruzioni, registro* registri[], int numero_registri)
{
  registro* begin = registri[0];
  registro* end = registri[1];

  //recupero l'id
  char id_ric[50];
	primo(istruzioni, id_ric, TRUE);
	int id_comp = atoi(id_ric);

  if( id_comp == id || id_comp == ID_UNIVERSALE ) //se è l'azione è per me
  {
    char interruttore[50];
    char nuovo_valore[20];

    primo(istruzioni, interruttore, TRUE); //recupero l'interruttore da gestire
    primo(istruzioni, nuovo_valore, TRUE); //recupero il valore

    if(strcmp(interruttore, "BEGIN") == 0) //se devo agire sul registro begin
    {
      begin -> valore.integer = atoi(nuovo_valore);
      signal(SIGALRM, gestisci_begin); //da fare --> l agestisci begin gestirà la end
      alarm(begin -> valore.integer); //in secondi
      //alarm => fra tot tempo fai qualcosa
    }
    else if(strcmp(interruttore, "END") == 0) //se devo agire sul registro end
    {
      end -> valore.integer = atoi(nuovo_valore);
      //farà qualcosa al figlio
    }

  }





	/*	// Prendo il nome dell'interruttore e la nuova posizione
		  char label[50];
  		primo(istruzioni, label, TRUE);
	    char pos[50];
	    primo(istruzioni, pos, TRUE);

	    //creo il messaggio
	    char msg[200];
  		sprintf(msg, "%s %d %s %s", UPDATE_LABEL, ID_UNIVERSALE, label, pos);

	    //Invio il messaggio al mio unico figlio se esiste la pipe
	    if (strcmp(pipe_figlio, "")!= 0)
      {
	    	send_msg(pipe_figlio, msg);
	    }

		}

	send_msg(pipe_interna, "DONE");
  */

}


//restituisce True se l'id è il proprio altrimenti interroga il figlio se è il suo id
void gestisci_ID(coda_stringhe* istruzioni)
{
  // Recupero l'ID e rispondo se è il mio o no.
  char id_ric[20];
  primo(istruzioni, id_ric, TRUE);
  int id_comp = atoi(id_ric);

  //se l'id è il mio -> rispondo TRUE (= sono io)
  if( id_comp == id || id_comp == ID_UNIVERSALE )
  {
    send_msg(pipe_interna, "TRUE");
  }
  else //se non sono io devo chiedere a mio figlio se lid è il suo
  {
    if(strcmp(pipe_figlio, "") != 0) //se filgio esiste
    {
      char msg[20];
      char res[20];
      sprintf(msg, "%s %s", ID, id_ric);

      send_msg(pipe_figlio, msg); //invio messaggio al figlio
      read_msg(pipe_figlio, res); //aspetto risposta del filgio

      send_msg(pipe_interna, res); //invio la risposta ricevuta nella pipe interna

    }

  }

}
