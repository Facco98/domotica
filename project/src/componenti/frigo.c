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

//funzione che calcola il valore stringa di un registro, FALSE in caso di error, TRUE altrimenti
boolean calcola_registro_stringa( const registro* registro, string res);

//funzione per calcolare il valore di un registro, in particolare quello relativo al tempo di utilizzo
boolean calcola_registro_intero( const registro* registro, int* res);

//funzione che gestisce i tre processi che costituiscono il componente (frigo)
void crea_processi_supporto(registro* registri[], int numero_registri, boolean* aperto);

//funzione che verifica sul mio id
void gestisci_ID(coda_stringhe* istruzioni);

//funzione per terminare processo/i del frigo
void termina(int x);

//funzione per modificare lo stato degli interruttori
void gestisci_LABELUP(coda_stringhe* istruzioni, registro* registri[], boolean* stato);

//funzione per gestire la richiesta dello stato del frigo
void gestisci_STATUSGET(coda_stringhe* istruzioni, registro* registri[], int numero_registri, boolean* stato);

//funzione per chiudere il frigo (da utilizzare se è passato troppo tempo)
void chiuditi_alarm(int x);

//funzione per interpretare messaggi
void ascolta_e_interpreta(registro* registri[], int numero_registri, boolean* aperto);


long apertura; //indica quanto il frigo è stato aperto
int id; //id del frigo
char pipe_interna[50]; //pipe per comunicare all'interno del frigo
char pipe_esterna[50];  //pipe per comunicare con l'umano
pid_t figli[2]; //memorizzo i process_id dei figli (i processi che costituiscono il frigo)
boolean stato; //stato del frigo, interruttore per l'apertura/chiusura
registro* registri[4]; //registri del frigo, sono 4: tempo di utilizzo, tempo dopo cui il frigo si richiude automaticamente, percentuale di riempimento e temperatura interna

char* a;

int main (int argn, char** argv)
{
  //stato del frigo e interruttore che ne indica l'apertura/chiusura
  stato = FALSE; //indica se il frigo è aperto o chiuso. Se il frigo è aperto allora è impostato a TRUE, altrimenti a FALSE

  //registri (totale 4)
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
  chiusura.valore.integer = 10; //imposto il valore di default pari a 10 secondi
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

//imposto quattro registri ai quattro appena creati
  int numero_registri = 4;
  registri[0] = &tempo_utilizzo;
  registri[1] = &chiusura;
  registri[2] = &riempimento;
  registri[3] = &temperatura;

  //controllo se argomenti input sono meno di due: nomefile(argv[0]) id(argv[1]) stato(argv[2]) tempo_di_utilizzo(argv[3]) delay(argv[4]) riempimento (argv[5]) temperatura (argv[6])
  if( argn < 2 )
  {
    exit(0);
  }

  //recupero l'id
  id = strtol(argv[1], &a, 10);
  if( id == 0 )
    exit(0);
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
  	registri[0] -> valore.integer = strtol(argv[3], &a, 10);
  }
	//recupero registri[1] ossia chiusura
   if (argn >=  5){
  	registri[1] -> valore.integer = strtol(argv[4], &a, 10);
  }
	//recupero registri[2] ossia riempimento
   if (argn >=  6){
  	registri[2] -> valore.integer = strtol(argv[5], &a, 10);
  }
	//recupero registri[3] ossia temperatura
   if (argn >=  7){
  	registri[3] -> valore.integer = strtol(argv[6], &a, 10);
  }

// se il frigo è aperto imposto la chiusura al valore del registro di chiusura
  if (stato == TRUE){
  	alarm(registri[1]->valore.integer);		//in secondi
  }

  //salvo i percorsi delle pipe per la comunicazione interna ed esterna
  sprintf(pipe_interna, "%s/%d_int", (string) PERCORSO_BASE_DEFAULT, id);
  sprintf(pipe_esterna, "%s/%d_ext", (string) PERCORSO_BASE_DEFAULT, id);

  //gestisco i processi che costituiscono il frigo
  crea_processi_supporto(registri, numero_registri, &stato);

}


boolean calcola_registro_stringa( const registro* registro, string res )
{
  return TRUE;
}

boolean calcola_registro_intero( const registro* registro, int* res )
{
//se non serve calcolare il registro o se non è un registro intero, restituisco FALSE
  if( registro -> da_calcolare == FALSE || registro -> is_intero == FALSE )
    return FALSE;

  //se il registro si chiama "time", calcolo il tempo di utilizzo
  if( strcmp(registro -> nome, "time") == 0 ){
    long ora = (long) time(NULL);
    *res = (registri[0] -> valore.integer + (ora) - apertura);
  }

  //se il registro si chiama "delay", restituisco il registro di ritardo di chiusura
    if( strcmp(registro -> nome, "delay") == 0 ){
    *res = (registri[1] -> valore.integer );
  }

//se il registro si chiama "perc", restituisco il registro di percentuale di riempimento
    if( strcmp(registro -> nome, "perc") == 0 ){
    *res = (registri[2] -> valore.integer );
  }

//se il registro si chiama "temp", restituisco il registro di temperatura interna
    if( strcmp(registro -> nome, "temp") == 0 ){
    *res = (registri[3] -> valore.integer );
  }

  return TRUE;
}

void crea_processi_supporto(registro* registri[], int numero_registri, boolean* aperto)
{

  crea_pipe(id, (string) PERCORSO_BASE_DEFAULT); //creo pipe per comunicazione con il dispositivo padre
  pid_t pid = fork(); //genero un processo identico a me
  if( pid == 0 ) //se sono il figlio
  {
    while(1) //continuo a trasferire messaggi da pipe_esterna a pipe_interna
    {
      char msg[200];

      read_msg(pipe_esterna, msg, 200);
      sem_wait(sem);
      send_msg(pipe_interna, msg);
      read_msg(pipe_interna, msg, 199);
      sem_post(sem);
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
        char msg[200];
        leggi_messaggio(id, (string) PERCORSO_BASE_DEFAULT, msg, 199);  //legge messaggio da pipe con il padre
        sem_wait(sem); //aspetto il semaforo
        send_msg(pipe_interna, msg); //scrive sulla pipe interna il messaggio preso dal padre
        read_msg(pipe_interna, msg, 199); //legge dalla pipe interna
        if( strcmp(msg, "DONE") != 0 ) //invio sulla pipe_padre tutto ciò che non è "DONE"
        {
          manda_messaggio(id, (string) PERCORSO_BASE_DEFAULT, msg);
        }
        sem_post(sem);  //aspetto il semaforo
      }

    }
    else if( pid > 0 ) //se sono il padre
    {
      figli[1] = pid; //memorizzo process-id del figlio sopra generato (seconda fork)
      signal(SIGINT, termina); //muoio se necessario
      signal(SIGALRM, chiuditi_alarm);		//se ricevo un SIGALARM, mi chiudo se necessario
      while(1) //resto in ascolto su pipe interna e interpreto
      {
        //gestisce i messaggi in arrivo
        ascolta_e_interpreta(registri, numero_registri, aperto);
      }

    }

  }

}

void ascolta_e_interpreta(registro* registri[], int numero_registri, boolean* aperto)
{
  //memorizzo i registri (variabili globali)
  registro* tempo_utilizzo = registri[0];
  registro* chiusura = registri[1];
  registro* riempimento = registri[2];
  registro* temperatura = registri[3];

  //resto in ascolto sulla pipe interna
  char messaggio[100];
  while( read_msg(pipe_interna, messaggio, 99) == FALSE) //leggo messaggio, se non riesco a leggerlo mando un perror
  {
    perror("Errore in lettura");
  }

  //elimino "\n" finale dal messaggio
  strtok(messaggio, "\n");

  //creo la coda di stringhe dal messaggio
  coda_stringhe* istruzioni = crea_coda_da_stringa(messaggio, " ");

  //recupero il nome del comando
  char nome_comando[20];
  if(!primo(istruzioni, nome_comando, TRUE) ) //se estraggo da coda vuota, messaggio di errore
  {
    exit(0);
  }

  //gestisco il comando
  if( strcmp(nome_comando, GET_STATUS) == 0 )
  {
    gestisci_STATUSGET(istruzioni, registri, numero_registri, aperto); //se il comando vuole recuperare lo stato del frigo chiamo questa funzione
  }
  else if( strcmp(nome_comando, UPDATE_LABEL) == 0 ) // se il comando vuole aggiornare interruttori chiamo questa funzione
  {
    gestisci_LABELUP(istruzioni, registri, aperto);
  }
  else if( strcmp(nome_comando, REMOVE) == 0 ) //se il comando vuole rimuovere il dispositivo lo rimuovo
  {
    char tmp[20];
    primo(istruzioni, tmp, TRUE);
    int id_ric = strtol(tmp, &a, 10);
    if( id_ric == id || id_ric == ID_UNIVERSALE )
    {
      termina(0);
    }
    send_msg(pipe_interna, "TRUE");

  }
  else if( strcmp(nome_comando, ID) == 0 ) //verifica se è il mio id
  {
    gestisci_ID(istruzioni);
  }
  else if(strcmp(nome_comando, "CONFIRM") == 0)	//verifica se è il mio id
  {
    gestisci_ID(istruzioni);
  }
  else if (strcmp(nome_comando, "SET_FILL") ==  0)
  { //controllo se devo impostare il riempimento
    char id_c[20];
    primo(istruzioni, id_c, TRUE);
    if( strtol(id_c, &a, 10) == id || strtol(id_c, &a, 10) == ID_UNIVERSALE ){
      char pos[50];
      primo(istruzioni, pos, TRUE);
      int perc = strtol(pos, &a, 10);
      if( perc <= 100 && perc >= 0 )
        riempimento->valore.integer = perc;
    } //imposto il riempimento del frigo al riempimento voluto
    send_msg(pipe_interna, "DONE");
  }
  else //qualsiasi altro comando non supportato
  {
    send_msg(pipe_interna, "DONE"); //invio DONE sulla pipe interna
  }
  distruggi(istruzioni); //elimino il comando arrivato



}

// funzione risponde alla domanda: posso raggiungere il dispositivo con ID id passando da te?
//per i dispositivi foglia (come il frigo) funziona in questo modo

void gestisci_ID(coda_stringhe* istruzioni)
{
  char tmp[10];
  primo(istruzioni, tmp, TRUE); //ricavo l'id dal messaggio

  if( strtol(tmp, &a, 10) == id ) //se l'id corrisponde al mio allora invio TRUE sula pipe interna
  {
    send_msg(pipe_interna, "TRUE");
  }
  else
  {
    send_msg(pipe_interna, "FALSE"); // altrimenti invio FALSE sula pipe interna
  }
}

void termina(int x)
{
  //uccido i processi figli che costituiscono il frigo
  kill(figli[0], SIGKILL);
  kill(figli[1], SIGKILL);
  close(file);
  ripulisci(id, (string) PERCORSO_BASE_DEFAULT);
  exit(0); //mi chiudo

}

//funzione per gestire l'aggiornamento per gli interruttori
void gestisci_LABELUP(coda_stringhe* istruzioni, registro* registri[], boolean* stato){

  //recupero i registri
  registro* tempo_utilizzo = registri[0];
  registro* chiusura = registri [1];
  registro* riempimento = registri[2];
  registro* temperatura = registri[3];

  boolean res = FALSE;
  char id_comp[20];
  primo(istruzioni, id_comp, TRUE); //recupero l'id indicato nel messaggio
  int id_ric = strtol(id_comp, &a, 10);

   //se il comando è indirizzato al frigo (l'id è il mio) agisco di conseguenza
  if( id_ric == id || id_ric == ID_UNIVERSALE ){
    char azione[20]; //azione da compiere sul frigo (= OPEN | CLOSE)
    primo(istruzioni, azione, TRUE); //estraggo dalla coda di stringhe l'azione da compiere

    char pos[20];  //nuova posizione dell'interruttore interessato
    primo(istruzioni, pos, TRUE); //recupero dalla coda di stringhe la nuova posizione dell'interruttore

     if(strcmp(azione, "APERTURA") == 0 && strcmp(pos, "ON") == 0)//se devo aprire il frigo
      {
        if(*stato == FALSE)//se il frigo è CHIUSO
        {
          *stato = TRUE; //"Apro" il frigo
          apertura = (long) time(NULL); //salvo l'ora in cui ho aperto il frigo
          tempo_utilizzo -> da_calcolare = TRUE;
          alarm(chiusura -> valore.integer); //tra x valore del registro chiusura mi arriva un SIGALARM
          res = TRUE;
        }
      }
      else if(strcmp(azione, "APERTURA") == 0 && strcmp(pos, "OFF") == 0) //se devo chiudere il frigo
      {
        if(*stato == TRUE) //se il frigo è APERTO
        {
          *stato = FALSE; //"chiudo" il frigo
          //salvo il l'intervallo di tempo che è rimasto aperto
          long ora = (long) time(NULL);
          tempo_utilizzo -> valore.integer += (ora - apertura);
          tempo_utilizzo -> da_calcolare = FALSE;
          alarm(0); //cancella l'alarm di chiusura se inviato precedentemente
          res = TRUE;
        }
      }
    else if (strcmp(azione, "TEMPERATURE") ==  0)
    { //controllo se devo impostare la temperatura
      temperatura->valore.integer = strtol(pos, &a, 10);  //imposto la temperatura del frigo alla temperatura voluta
      res = TRUE;
    }
    else if (strcmp(azione, "DELAY") == 0 && strtol(pos, &a, 10) >= 1) { // se devo modificare il ritardo di chiusura e questo è maggiore di 1
      chiusura -> valore.integer = strtol(pos, &a, 10); //imposto il registro chiusura al valore voluto
      if( chiusura -> valore.integer >= 0 && *stato == TRUE ){
        alarm(0); //cancella l'alarm di chiusura se inviato precedentemente
        alarm(chiusura -> valore.integer); //invia un alarm di chiusura al nuovo valore desiderato
        res = TRUE;

      }
    }
  }

  //rispondo sulla pipe interna di aver concluso (o non concluso) l'azione desiderata
  if( res == TRUE )
    send_msg(pipe_interna, "TRUE");
  else
    send_msg(pipe_interna, "FALSE");
}

//funzione per rispondere con lo stato di tutti i registri
void gestisci_STATUSGET(coda_stringhe* istruzioni, registro* registri[], int numero_registri, boolean* stato){
  char indice_ric[10];
  primo(istruzioni, indice_ric, TRUE); //recupero l'id del dispositivo interessato
  int indice = strtol(indice_ric, &a, 10);

  if( indice == ID_UNIVERSALE || indice == id ) //se sono io il dispositivo prescelto
  {
    int i = 0;
    char res[1024*2];
    //preparo il messaggio di risposta in cui indico il mio stato
    sprintf(res, "%s fridge %d %s", GET_STATUS_RESPONSE, id, *stato == TRUE ? "OPEN" : "CLOSE" ); //risponde il proprio stato (aperto o chiuso)
    for( i = 0; i < numero_registri; i++ ) // stampa i registri
    {
      char str[1024];
      stampa_registro(registri[i], str);
      strcat(res, " ");
      strcat( res, str );
    }

    send_msg(pipe_interna, res); //invio la risposta sulla pipe interna ( che verrà poi inviata alla pipe con il padre dal processo assegnato)
  } else
    send_msg(pipe_interna, "DONE");
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
