#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>

#include "strutture_dati/tipi_componente.h"
#include "strutture_dati/coda_stringhe.h"
#include "comunicazione/comunicazione.h"

typedef boolean b;

boolean calcola_registro_stringa( const registro* registro, string res );
boolean calcola_registro_intero( const registro* registro, int* res );
//gestisce i tre processi che costituiscono il componente (frigo)
void crea_processi_supporto(registro* registri[], int numero_registri, boolean* aperto, boolean* apri, boolean* chiudi);
//verifica sul mio id
void gestisci_ID(coda_stringhe* istruzioni);
//terminare processo/i del frigo
void termina(int x);
//modificare lo stato degli interruttori
void gestisci_LABELUP(coda_stringhe* istruzioni, registro* registri[], boolean* stato, boolean* apri, boolean* chiudi);
//funzione per gestire la richiesta dello stato del frigo
void gestisci_STATUSGET(coda_stringhe* istruzioni, registro* registri[], int numero_registri, boolean* stato);
//funzione per chiudere il frigo (da utilizzare se è passato troppo tempo)
void chiuditi_alarm(int x);
void ascolta_e_interpreta(registro* registri[], int numero_registri, boolean* aperto, boolean* apri, boolean* chiudi);


long apertura;

int id;

char pipe_interna[50];
char pipe_esterna[50];

pid_t figli[2];
boolean stato;
registro* registri[4];


int main (int argn, char** argv)
{
  //stato
  stato = FALSE; //indica se il frigo è aperto o chiuso
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

  //delay, registro chiusura: tempo dopo cui si richiude automaticamente
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

//i quattro registri
  int numero_registri = 4;
  registri[0] = &tempo_utilizzo;
  registri[1] = &chiusura;
  registri[2] = &riempimento;
  registri[3] = &temperatura;

  //controllo se argomenti input sono meno di due
  if( argn < 2 )
  {
    exit(130);
  }

  //recupero l'id
  id = atoi(argv[1]);
// se gli argomenti sono più di 3 allora posso mettere i valori in input, sovrascritti a quelli di default
  //in ordine:
  //recupero lo stato
  if( argn >= 3 ){
    if( strcmp(argv[2], "ON") == 0 ){
      apertura = (long) time(NULL);
      tempo_utilizzo.da_calcolare = TRUE;
      stato = TRUE;
    }
    else
      stato = FALSE;
  }

  //recupero registri[0] ossia tempo_utilizzo
  if (argn >=  4){
  	registri[0] -> valore.integer = atoi(argv[3]);
  }
	//recupero registri[1] ossia chiusura
   if (argn >=  5){
  	registri[1] -> valore.integer = atoi(argv[4]);
  }
	//recupero registri[2] ossia riempimento
   if (argn >=  6){
  	registri[2] -> valore.integer = atoi(argv[5]);
  }
	//recupero registri[3] ossia temperatura
   if (argn >=  7){
  	registri[3] -> valore.integer = atoi(argv[6]);
  }

  if (stato == TRUE){
  	alarm(registri[1]->valore.integer);		//in secondi
  }

  //salvo i percorsi delle pipe per la comunicazione interna ed esterna
  sprintf(pipe_interna, "%s/%d_int", (string) PERCORSO_BASE_DEFAULT, id);
  sprintf(pipe_esterna, "%s/%d_ext", (string) PERCORSO_BASE_DEFAULT, id);

  //gestisco i processi che costituiscono il frigo
  crea_processi_supporto(registri, numero_registri, &stato, &apri, &chiudi);

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
    *res = (registro -> valore.integer + (ora) - apertura);

  }
  return TRUE;
}

void crea_processi_supporto(registro* registri[], int numero_registri, boolean* aperto, b* apri, b* chiudi)
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
      crea_pipe(id, (string) PERCORSO_BASE_DEFAULT); //creo pipe per comunicazione con il dispositivo padre
      while(1) //leggo e invio messaggi da pipe con padre (= dispositivo sopra) a pipe_interna
      {
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
    else if( pid > 0 ) //se sono il padre
    {
      figli[1] = pid; //memorizzo process-id del figlio sopra generato (seconda fork)
      signal(SIGINT, termina); //muoio se necessario
      signal(SIGALRM, chiuditi_alarm);		//se ricevo un SIGALARM, mi chiudo se necessario
      mkfifo(pipe_interna, 0666); //genero pipe_interna
      while(1) //resto in ascolto su pipe interna e interpreto
      {
        ascolta_e_interpreta(registri, numero_registri, aperto, apri, chiudi);
      }

    }

  }

}

void ascolta_e_interpreta(registro* registri[], int numero_registri, boolean* aperto, boolean* apri, boolean* chiudi)
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
  if(!primo(istruzioni, nome_comando, TRUE) )
  {
    exit(140);
  }

  //gestisco il comando
  if( strcmp(nome_comando, GET_STATUS) == 0 )
  {
    gestisci_STATUSGET(istruzioni, registri, numero_registri, aperto); //recuperare lo stato del frigo CONTROLLARE SE CORRETTA
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
  else if(strcmp(nome_comando, "CONFIRM") == 0)	//verifica se è il mio id
  {
    gestisci_ID(istruzioni);
  }
  else
  {
    printf("Comando non supportato: %s\n", nome_comando); //OK
    send_msg(pipe_interna, "DONE");
  }




}

// risponde alla domanda: posso  raggiungere il dispositivo con ID id passando da te?
//per i dispositivi foglia è identica a prima: se sono io true, altrimenti false

void gestisci_ID(coda_stringhe* istruzioni)
{
  char tmp[10];
  primo(istruzioni, tmp, TRUE);
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
                                              //in questi casi mi comporto come fossi una finestra
      * posso dovermi chiudere automaticamente 			          //fatto, bisogna capire quando si modifica questo valore (delay)
      *posso dover regolare il termostato                         //OK
      *
      */
//Se è un Hmessaggio tolgo la H dall'inizio e procedo normalmente
    if (pos[0] == "H"){
    	*pos += 1;
    }

    //in seguito il codice per aprire o chiudere il frigo (tramite comando), la chiusura automatica non è ancora implementata né la gestione del termostato
     if(strcmp(azione, "OPEN") == 0 && strcmp(pos, "ON") == 0)//se devo aprire il frigo
      {
        if(*stato == FALSE)//se il frigo è CHIUSO
        {
          *apri = TRUE; //"schiaccio" interruttore di apertura
          *stato = TRUE; //"Apro" il frigo
          apertura = (long) time(NULL); //salvo l'ora in cui ho aperto il frigo
          tempo_utilizzo -> da_calcolare = TRUE;
          *apri = FALSE; //interruttore torna su off
          alarm(chiusura -> valore.integer); //tra x valore del registro chiusura mi arriva un SIGALARM
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
          alarm(0); //cancella l'alarm di chiusura se inviato precedentemente
        }
      }
  else if (strcmp(azione, "SET_TEMPERATURE") ==  0)
  { //controllo se devo impostare la temperatura
    temperatura->valore.integer = atoi(pos);  //imposto la temperatura del frigo alla temperatura voluta
  }
// 			???????????????????????????????????????????????
    else if (strcmp(azione, "HSET_FILL") ==  0)
    /*qua ho messo l'H all'inizio perché questo è solo override manuale,
    il comando per cambiare il riempimento da mandare come umano è HHSET_FILL
    in questo modo gli altri dispositivi non possono mandare questo messaggio, neanche volendo e il prof non può rompere il codice?
    Alternativa (penso sia quella corretta):
    gli altri dispositivi in ogni caso NON possono mandare questo messaggio (non è nei tipi supportati) quindi posso
    togliere la H dall'inizio?

    	*/
  { //controllo se devo impostare il riempimento
    riempimento->valore.integer = atoi(pos);  //imposto il riempimento del frigo al riempimento voluto
  }
  else
  {
    distruggi_coda(istruzioni); //elimino il messaggio arrivato

  }
  send_msg(pipe_interna, "DONE"); //rispondo sulla pipe interna di aver fatto (= niente perchè no sono io il processo interessato)

}


}

void gestisci_STATUSGET(coda_stringhe* istruzioni, registro* registri[], int numero_registri, boolean* stato){
//copia-incollata (praticamente) dalla finestra, l'implementazione è la stessa, praticamente, perché rispondo con lo stato di tutti i registri
  char indice_ric[10];
  primo(istruzioni, indice_ric, TRUE); //recupero l'id del dispositivo interessato
  int indice = atoi(indice_ric);

  if( indice == ID_UNIVERSALE || indice == id ) //se sono io il dispositivo prescelto
  {
    int i = 0;
    char res[1024*2];
    sprintf(res, "%s fridge %d %s", GET_STATUS_RESPONSE, id, *stato == TRUE ? "OPEN" : "CLOSE" ); //risponde il proprio stato (aperto o chiuso)
    for( i = 0; i < numero_registri; i++ ) // stampa i registri
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

	registro* tempo_utilizzo = registri[0]; //registro tempo_utilizzo

	stato = FALSE; //"chiudo" il frigo
	//salvo il l'intervallo di tempo che è rimasto aperto
	long ora = (long) time(NULL);
	tempo_utilizzo -> valore.integer += (ora - apertura);
	tempo_utilizzo -> da_calcolare = FALSE;
	alarm(0); //cancella l'alarm di chiusura se inviato precedentemente

}


/*Commenti, dubbi, perplessità, note
------------------------------------------------------TODO-----------------------------------------------------
Override manuale per il registro di riempimento

------------------------------------------------------Cose implementate ma da controllare:-------------------------------------
Implementata la funzione  gestisci_STATUSGET (da testare).
Implementata la funzione  gestisci_LABELUP (da testare).
Implementata void chiuditi_alarm(int x): chiude (da usare se arriva sigalarm).
Come gestire il registro temperatura.
Come gestire il registro di chiusura (delay).
*/
