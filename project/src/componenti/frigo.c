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
//modificare lo stato degli interruttori
void gestisci_LABELUP(coda_stringhe* istruzioni, registro* registri[], boolean* stato, boolean* apri, boolean* chiudi, int temperatura);
//funzione per gestire la richiesta dello stato del frigo
void gestisci_STATUSGET(coda_stringhe* istruzioni, registro* registri[], int numero_registri);
//funzione per chiudere il frigo (da utilizzare se è passato troppo tempo)
void chiuditi_alarm(int x);

long apertura;

int id;

char pipe_interna[50];
char pipe_esterna[50];

pid_t figli[2];


int main (int argn, char** argv)
{
  //stato
  boolean aperto = FALSE; //indica se il frigo è aperto o chiuso
  boolean apri = FALSE;
  boolean chiudi = FALSE;
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
  registro chiusura;
  strcpy(chiusura.nome, "delay");
  chiusura.da_calcolare = FALSE; 
  chiusura.valore.integer = 0; 
  chiusura.is_intero = TRUE; 
  

  //percentuale di riempimento
  registro riempimento;
  strcpy(riempimento.nome, "perc");
  riempimento.da_calcolare = FALSE; 
  riempimento.valore.integer = 0; 
  riempimento.is_intero = TRUE; 
  

  //temperatura interna
  registro temperatura;
  strcpy(temperatura.nome, "temp");
  temperatura.da_calcolare = FALSE;
  temperatura.valore.integer = 0; 
  temperatura.is_intero = TRUE; 
  

  int numero_registri = 4;
  registro* registri[] = {&tempo_utilizzo, &chiusura, &riempimento, &temperatura};

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
    figli[0] = pid; //memorizzo il process-id del figlio appena generato (fork sopra)
    pid = fork(); //genero un nuovo processo identico a me
    if( pid == 0 ) //se sono il figlio
    {
      while(1) //leggo e invio messaggi da pipe con padre (= dispositivo sopra) a pipe_interna
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
      signal(SIGALARM, chiuditi_alarm);
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
  //qua è corretto mettere tutti i registri, così posso controllare se il tempo di utilizzo > chiusura allora chiudo il frigo?
  registro* tempo_utilizzo = registri[0];
  registro* chiusura = registri[1];
  registro* riempimento = registri[2];
  registro* temperatura = registri[3];


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
    gestisci_STATUSGET(istruzioni, registri, numero_registri); //recuperare lo stato del frigo CONTROLLARE SE CORRETTA
  }
  else if( strcmp(nome_comando, UPDATE_LABEL) == 0 ) //aggiornare interruttori CONTROLLARE SE CORRETTA
  {
    gestisci_LABELUP(istruzioni, registri, aperto, apri, chiudi);  
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

//funzione per gestire l'aggiornamento per gli interruttori
void gestisci_LABELUP(coda_stringhe* istruzioni, registro* registri[], boolean* stato, boolean* apri, boolean* chiudi){
  //recupero i registri 

  registro* tempo_utilizzo = registri[0];
  registro* chiusura = registri [1];
  registro* riempimento = registri[2];
  registro* temperatura = registri[3];

  char id_comp[20];
  primo(istruzioni, id_comp, TRUE); //recupero l'id indicato nel messaggio, se sono io faccio robe
  int id_ric = atoi(id_comp);
  // cosa fare se sono io che devo fare le cose, effettivamente
  if( id_ric == id || id_ric == ID_UNIVERSALE ){
    char azione[20]; //azione da compiere sul frigo (= OPEN | CLOSE)
    primo(istruzioni, azione, TRUE); //estraggo dalla coda di stringhe l'azione da compiere

    char pos[20];
    primo(istruzioni, pos, TRUE);
    /*a questo punto possono succedere diverse cose
      * posso aprire il frigo                                     //OK
      *posso chiudere il frigo                                    //OK
                                              //in questi casi mi comporto come fossi una finestra, circa?
      * posso dovermi chiudere automaticamente (?)                //da fare
      *posso dover regolare il termostato                         //OK
      *
      */
    //in seguito il codice per aprire o chiudere il frigo (tramite comando), la chiusura automatica non è ancora implementata né la gesione del termostato
     if(strcmp(azione, "OPEN") == 0 && strcmp(pos, "ON") == 0)//se devo aprire il frigo
      {
        if(*stato == FALSE)//se il frigo è CHIUSO
        {
          *apri = TRUE; //"schiaccio" interruttore di apertura
          *stato = TRUE; //"Apro" il frigo
          apertura = (long) time(NULL); //salvo l'ora in cui ho aperto il frigo
          tempo_utilizzo -> da_calcolare = TRUE; // ????
          *apri = FALSE; //interruttore torna su off
        }
      }
      else if(strcmp(azione, "CLOSE") == 0 && strcmp(pos, "ON") == 0)
      {
        if(*stato == TRUE) //se il frigo è APERTO
        {
          *chiudi = TRUE; //"schiaccio" l'interruttore per chiudere il frigo
          *stato = FALSE; //"chiudo" il frigo
          //salvo il l'intervallo di tempo che è rimasto aperto
          long ora = (long) time(NULL);
          tempo_utilizzo -> valore.integer += (ora - apertura);
          tempo_utilizzo -> da_calcolare = FALSE;
        }
      }

  
  else if (strcmp(azione, "SET_TEMPERATURE") ==  0)
  { //controllo se devo impostare la temperatura
    temperatura->valore.integer = atoi(pos);  //imposto la temperatura del frigo alla temperatura voluta
  }
  else
  {
    distruggi_coda(istruzioni); //elimino il messaggio arrivato

  }
  send_msg(pipe_interna, "DONE"); //rispondo sulla pipe interna di aver fatto (= niente perchè no sono io il processo interessato)

}

  


}

void gestisci_STATUSGET(coda_stringhe* istruzioni, registro* registri[], int numero_registri){
//copia-incollata (praticamente) dalla finestra, l'implementazione è la stessa, praticamente, perché rispondo con lo stato di tutti i registri
  char indice_ric[10];
  primo(istruzioni, indice_ric, TRUE); //recupero l'id del dispositivo interessato
  int indice = atoi(indice_ric);

  if( indice == ID_UNIVERSALE || indice == id ) //se sono io il dispositivo prescelto
  {
    int i = 0;
    char res[1024*2];
    sprintf(res, "%s FRIDGE, id: %d ", GET_STATUS_RESPONSE, id );
    for( i = 0; i < numero_registri; i++ )
    {
      char str[1024];
      stampa_registro(registri[i], str);
      strcat(res, " ");
      strcat( res, str );
    }

    send_msg(pipe_interna, res); //invio la risposta sulla pipe interna ( che verrà poi inviata alla pipe con il padre dal processo assegnato)
  }
}


void chiuditi_alarm(int x){
//int x è il tipo di segnale
//Se arriva un sigalarm se sono aperto mi chiudo
  //ti colleghi alla pipe interna e dici di chiudersi
  char messaggio_chiusura[200];
  sprintf (messaggio_chiusura, id, " CLOSE ON");
  send_msg(pipe_interna, messaggio_chiusura);
    
}


/*Commenti, dubbi, perplessità, note
------------------------------------------------------COSE DA IMPLEMENTARE -----------------------------------------------------
Come gestire il registro riempimento? 
Come gestire il registro di chiusura (delay)?



------------------------------------------------------Cose implementate ma da controllare:-------------------------------------

Implementate le funzioni  gestisci_STATUSGET e gestisci_LABELUP (da testare).
Implementata void chiuditi_alarm(int x): manda un messaggio di chiusura (da usare se arriva sigalarm).

La chiamata di gestisci_LABELUP nella riga 200 probabilmente è da modificare, se gestisci_LABELUP è implementata in modo corretto

L'implementazione di gestisci_STATUSGET è la stessa della finestra?
//CORRETTO




----------------------------------------------------Cose implementate, ma forse fanno schifo:------------------------------------- 
Devo anche implementare il fatto che se è aperto da tropo tempo deve essere in qualche modo chiuso? Come? Dove, in while (1) nel main, probabilmente
"troppo tempo" <- indicato dal registro delay, per noi chiusura : implementato in ascolta_e_interpreta, da controllare

In ascolta e interpreta ho messo tutti i registri per due motivi:
- controllare se tempo_utilizzo > chiusura e chiudere il frigo <--- IMPLEMENTATO, CONTROLLARNE LA CORRETTEZZA (line 230 o giù di lì)
l'ho messo in ascolta_e_interpreta perché deve farlo sempre e questa funzione è eseguita sempre nel main (while(1))
- controllare lo stato di riempimento e restituire lo stato di riempimento <- in status get in realtà, ma la restituisce comunque già
devo poter impostare il registro di chiusura (è modificabile), dove?
//SBAGLIATO, IMPLEMENTARE UNA FUNZIONE A PARTE




------------------------------------------------------Dubbi, cose da dover fare ma non so dove, nè come:----------------------------
Il registro riempimento può venir modificato solo manualmente, come si fa in codice questa roba? C'entra l'umano?
per controllare lo stato del registro c'è già in statusget (forse), per modificarlo manca, deve esserci da qualche parte ma boh
//DA CHIEDERE AL PROFESSORE GIOVEDI'

*/
