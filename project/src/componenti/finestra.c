#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>

#include "strutture_dati/tipi_componente.h"
#include "strutture_dati/coda_stringhe.h"
#include "comunicazione/comunicazione.h"

//funzione per calcolare il valore di un registro, in particolare quello relativo al tempo di utilizzo
boolean calcola_registro_intero( const registro* registro, int* res );

//funzione che calcola il valore stringa di un registro, FALSE in caso di error, TRUE altrimenti
boolean calcola_registro_stringa( const registro* registro, string res);

//funzione per gestire l'aggiornamento degli interruttori
void gestisci_LABELUP(coda_stringhe* istruzioni, registro* registri[], boolean* stato, boolean* apri, boolean* chiudi);

//funzione per gestire la richiesta dello stato della finestra
void gestisci_STATUSGET(coda_stringhe* istruzioni, registro* registri[], int numero_registri, boolean* stato);

//funzione per capire se l'id è quello del processo oppure no
void gestisci_ID(coda_stringhe* istruzioni);

//funzione per terminare il processo
void termina(int x);

//gestisce la creazione dei processi che costiutiscono il componente (finestra)
void crea_processi_supporto(registro* registri[], int numero_registri, boolean* stato, boolean* apri, boolean* chiudi);

//funzione per interpretare messaggi
void ascolta_e_interpreta (registro* registri[], int numero_registri, boolean* stato, boolean* apri, boolean* chiudi);


long apertura; //indica quando la finestra è stata aperta
int id; //id del processo
char pipe_interna[50]; //pipe per comunicare all'interno della finestra
char pipe_esterna[50]; //pipe per comunicare con l'umano

pid_t figli[2]; //memorizzo i process_id dei figli (i processi che costituiscono la finestra)

int main (int argn, char** argv)
{
  //stato
  boolean stato = FALSE; //indica lo stato (aperta = true, chiusa = false) della finestra

  //interruttori -> dopo essere stati "premuti" tornano subito a FALSE (= off)
  boolean apri_finestra = FALSE; //interruttore per aprire finestra
  boolean chiudi_finestra = FALSE; //interruttore per chiudere finestra

  //registro : indica il tempo di utilizzo, cioè quanto resta aperta
  registro tempo_utilizzo;
  strcpy(tempo_utilizzo.nome, "time"); //nome del registro = TIME
  tempo_utilizzo.da_calcolare = FALSE;
  tempo_utilizzo.valore.integer = 0;
  tempo_utilizzo.is_intero = TRUE;

  int numero_registri = 1;
  registro* registri[] = {&tempo_utilizzo};


  //gestione argomenti passati in input
  if(argn < 2) //troppo pochi argomenti -> do messaggio di Errore
  {
    exit(130);
  }

  //recupero l'id del componente che deve sempre essere passato per primo
  id = atoi(argv[1]);

  //FORMATO ARGOMENTI: nomefile(argv[0]) id(argv[1]) stato(argv[2]) tempo_di_utilizzo(argv[3])
  if(argn >= 3) //FORMATO INPUT : file id stato
  {
    if(strcmp(argv[2], "OPEN") == 0) //se la finestra è aperta aggiorno il registro
    {
      apertura = (long) time(NULL); //salvo tempo di apertura
      tempo_utilizzo.da_calcolare = TRUE;
      stato = TRUE; //apro la finetsra/aggiorno lo stato
    }
    else //se la finestra è chiusa, aggiorno lo stato
    {
      stato = FALSE; //chiudo la finestra
    }
  }

  //fino allo stato è giò stato salvato nell'if sopra, devo solo aggiornare il tempo di utilizzo
  if(argn >= 4) //FORMATO INPUT : file id stato tempo
  {
    string tempo = argv[3];
    tempo_utilizzo.valore.integer = atoi(tempo); //aggiorno il registro
  }


  //creo due pipe per gestire messaggi da centralina e da umano
  sprintf(pipe_interna, "%s/%d_int", (string) PERCORSO_BASE_DEFAULT, id);
  sprintf(pipe_esterna, "%s/%d_ext", (string) PERCORSO_BASE_DEFAULT, id);

  //gestisce la generazione dei processi che costituiscono la finestra
  crea_processi_supporto(registri, numero_registri, &stato, &apri_finestra, &chiudi_finestra); //aggiunti apri_finestra e chiudi_finestra perchè poi devono essere passati alla ascolta e interpreta


}

boolean calcola_registro_intero( const registro* registro, int* res )
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
    *res = (registro -> valore.integer + (ora) - apertura);
  }

  return TRUE;
}

boolean calcola_registro_stringa( const registro* registro, string res )
{
  return TRUE;
}

void ascolta_e_interpreta (registro* registri[], int numero_registri, boolean* stato, boolean* apri, boolean* chiudi)
{
  registro* tempo_utilizzo = registri[0];

  //quando arriva un messaggio, leggo, tolgo "\n" alla fine, e creo la coda di stringhe dalla stringa originale
  char messaggio[100];
  while(read_msg(pipe_interna, messaggio, 99) == FALSE) //leggo messaggio dalla pipe interna
    perror("Errore in lettura");

  strtok(messaggio, "\n"); //elimino "\n" finale

  coda_stringhe* istruzioni = crea_coda_da_stringa(messaggio, " "); //creo la coda di strignhe

  //inizio ad interpretare il messaggio arrivato
  char nome_comando[20];
  if(!primo(istruzioni, nome_comando, TRUE)) //se estraggo da coda vuota, messaggio di errore
  {
    exit(140);
  }

  //gestisco i vari comandi che possono arrivare
  if(strcmp(nome_comando, GET_STATUS) == 0) //se il comando serev per restituitìre lo STATO
  {
    gestisci_STATUSGET(istruzioni, registri, numero_registri, stato);
  }

  else if(strcmp(nome_comando, UPDATE_LABEL) == 0) //se il comando serve per AGGIORNARE UN INTERRUTTORE
  {
    gestisci_LABELUP(istruzioni, registri, stato, apri, chiudi);
  }

  else if(strcmp(nome_comando, REMOVE) == 0) //se il comando serve per RIMUOVERE la finestra
  {
    char tmp[20];
    primo(istruzioni, tmp, TRUE);
    int id_ric = atoi(tmp);
    if( id_ric == id || id_ric == ID_UNIVERSALE ){

      termina(0);

    }
    send_msg(pipe_interna, "TRUE");
  }
  //comando che chiede se il dispositivo con l'ID inviato sia raggiungibile attraverso la finestra
  else if(strcmp(nome_comando, ID) == 0)
  {
    gestisci_ID(istruzioni);
  }
  //comando che chiede se l'ID inviato corrisponda al proprio
  else if(strcmp(nome_comando, "CONFIRM") == 0)
  {
    gestisci_ID(istruzioni);
  }

  else //un qualsiasi altro comando non previsto
  {
    send_msg(pipe_interna, "DONE");
  }
  distruggi(istruzioni);

}


void gestisci_LABELUP(coda_stringhe* istruzioni, registro* registri[], boolean* stato, boolean* apri, boolean* chiudi)
{
  registro* tempo_utilizzo = registri[0];

  char id_comp[20];
  primo(istruzioni, id_comp, TRUE); //recupero l'id indicato nel messaggio
  int id_ric = atoi(id_comp);

  boolean res = FALSE;

  //se il comando è indirizzato alla finestra (l'id è il mio) agisco di conseguenza
  if( id_ric == id || id_ric == ID_UNIVERSALE )
  {
    char azione[20]; //azione da compiere sulla finetsra (= OPEN | CLOSE)
    primo(istruzioni, azione, TRUE); //estraggo dalla coda di stringhe l'azione da compiere

    char pos[20]; //nuova posizione dell'interruttore interessato
    primo(istruzioni, pos, TRUE); //recupero dalla coda di stringhe la nuova posizione dell'interruttore

      if(strcmp(azione, "OPEN") == 0 && strcmp(pos, "ON") == 0)//se devo APRIRE la finestra
      {
        if(*stato == FALSE)//se la finestra è CHIUSA
        {
          *apri = TRUE; //"schiaccio" interruttore di apertura
          *stato = TRUE; //"Apro" la finestra
          apertura = (long) time(NULL); //salvo l'ora in cui ho aperto la fienstra
          tempo_utilizzo -> da_calcolare = TRUE;
          *apri = FALSE; //interruttore torna su off
        }
        res = TRUE;
      }
      else if(strcmp(azione, "CLOSE") == 0 && strcmp(pos, "ON") == 0) //se deco CHIUDERE la finestra
      {
        if(*stato == TRUE) //se la finestra è APERTA
        {
          *chiudi = TRUE; //"schiaccio" l'interruttore per chiudere la finestra
          *stato = FALSE; //"chiudo" la fienstra
          //salvo il l'intervallo di tempo che è rimasta aperta
          long ora = (long) time(NULL);
          tempo_utilizzo -> valore.integer += (ora - apertura);
          tempo_utilizzo -> da_calcolare = FALSE;
        }
        res = TRUE;
      }

  }
  //rispondo sulla pipe interna di aver concluso l'azione
  if( res == TRUE )
    send_msg(pipe_interna, "TRUE");
  else
    send_msg(pipe_interna, "FALSE");

}

void gestisci_STATUSGET(coda_stringhe* istruzioni, registro* registri[], int numero_registri, boolean* stato)
{
  char indice_ric[10];
  primo(istruzioni, indice_ric, TRUE); //recupero l'id del dispositivo interessato
  int indice = atoi(indice_ric);

  if( indice == ID_UNIVERSALE || indice == id ) //se sono io il dispositivo prescelto
  {
    int i = 0;
    char res[1024*2];
    //preparo il messaggio di risposta in cui indico il mio stato
    sprintf(res, "%s window %d %s", GET_STATUS_RESPONSE, id, *stato == TRUE ? "OPEN" : "CLOSE" );
    for( i = 0; i < numero_registri; i++ ) //stampa il valore del registro
    {
      char str[1024];
      stampa_registro(registri[i], str);
      strcat(res, " ");
      strcat( res, str );
    }

    send_msg(pipe_interna, res); //invio la risposta sulla pipe interna ( che verrà poi inviata alla pipe con il padre dal processo assegnato)
  }

}

void gestisci_ID(coda_stringhe* istruzioni)
{
  char tmp[10];
  primo(istruzioni, tmp, TRUE); //ricavo l'id dal messaggio

  if( atoi(tmp) == id ) //se l'id è il mio
  {
    send_msg(pipe_interna, "TRUE"); //rispondo TRUE sulla pipe_interna (che verrà inviato alla pipe_padre)
  }
  else //se l'id non è il mio
  {
    send_msg(pipe_interna, "FALSE"); //rispondo FALSE sulla pipe_interna (che verrà inviato all apipe_padre)
  }

}

void termina(int x)
{

  kill(figli[0], SIGKILL); //uccidi primo figlio
  kill(figli[1], SIGKILL); //uccidi secondo figlio
  close(file);

  char pipe[50];
  sprintf(pipe, "/tmp/%d", id);
  unlink(pipe); //elimino la pipe con il padre
  unlink(pipe_esterna); //elimino pipe_esterna
  unlink(pipe_interna); //elimino pipe_interna
  exit(0); //mi chiudo

}

void crea_processi_supporto(registro* registri[], int numero_registri, boolean* stato, boolean* apri, boolean* chiudi)
{
  crea_pipe(id, (string) PERCORSO_BASE_DEFAULT); //creo la mia pipe destinata alle comunicazioni con il processo padre
  mkfifo(pipe_interna, 0666); //genero la pipe interna
  mkfifo(pipe_esterna, 0666); //genero la pipe esterna che comunica con l'umano

  pid_t pid = fork(); //genero un processo identico a me (finestra)
  if( pid == 0 ) //se sono il figlio (= processo appena generato)
  {
    while(1) //continua a copiare i messaggi da pipe esterna a pipe interna
    {
      char msg[200];
      read_msg(pipe_esterna, msg, 200);
      send_msg(pipe_interna, msg);
    }
  }

  else if( pid > 0 ) //se sono il padre (= processo generante)
  {
    figli[0] = pid; //mi salvo il process-id del processo appena generato all'if sopra (fork di prima)
    pid = fork(); //genero un nuovo processo identico a me (finestra)
    if( pid == 0 ) //se sono il figlio (= processo appena generato)
    {
      while(1)
      {
        char msg[200];
        leggi_messaggio(id, (string) PERCORSO_BASE_DEFAULT, msg, 199); //legge messaggio da pipe con il padre
        send_msg(pipe_interna, msg); //scrive sulla pipe interna il messaggio preso dal padre
        read_msg(pipe_interna, msg, 199); //legge dalla pipe interna
        if( strcmp(msg, "DONE") != 0 ) //se riceve "DONE" non fa nulla, qualsiasi altra cosa viene inviata sulla pipe con il padre
          manda_messaggio(id, (string) PERCORSO_BASE_DEFAULT, msg);
      }

    }

    else if( pid > 0 ) //se sono il padre (= processo generante)
    {
      figli[1] = pid; //mi salvo il process-id del figlio appena generato dalla fork sopra
      signal(SIGINT, termina); //se ricevo segnale di morte --> chiamo funzione termina

      while(1) //resto perennemente in ascolto sulla mia pipe interna
      {
        //gestisco i messaggi che mi arrivano
        ascolta_e_interpreta(registri, numero_registri, stato, apri, chiudi);
      }
    }

  }

}
