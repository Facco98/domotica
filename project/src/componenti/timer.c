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

//funzione che calcola il valore stringa di un registro, FALSE in caso di error, TRUE altrimenti
boolean calcola_registro_stringa( const registro* r, string output);

//termina il processo
void termina (int x);

//gestisce la creazione di una nuova componente
void gestisci_SPAWN(coda_stringhe* istruzioni);

//gestisce la richeista dello stato del componente collegato
void gestisci_STATUSGET(coda_stringhe* istruzioni);

//gestisce la rimozione dei componenti
void gestisci_REMOVE(coda_stringhe* istruzioni);

//gestisce i messaggi in arrivo al timer
void ascolta_e_interpreta(registro* registri[], int numero_registri);

//gestisce l'aggiornamento degli interruttori (sia del timer, che del componente a lui collegato)
void gestisci_LABELUP(coda_stringhe* istruzioni, registro* registri[], int numero_registri);

//gestisce la creazione di una nuova componente
void genera_figlio(coda_stringhe* status);

//risponde TRUE se è in grado di raggiungere il dispositivo con l'id passato
void gestisci_ID(coda_stringhe* istruzioni);

//calcola se c'è stato override manuale
boolean calcola_override(string str, lista_stringhe* tipi_figli, lista_stringhe* confronti);

//aggiorna gli stati attesi del figlio
void aggiorna_stati(string str);

//sostituisce ',' con spazi ' ' per separar ei figli di un hub
void decodifica_figli( string tmp );

//gestisce l'avvio del timer
void gestisci_begin(int x);

//gestisce lo spegnimento del timer
void gestisci_end(int x);

//sostituisce gli '_' con gli spazi ' ' all'interno di una stringa
void decodifica_controllo(string tmp);

char* a;


int id; //id del dispositivo

char pipe_figlio[50]; //memorizza la pipe del dispositivo a lui collegato

char pipe_interna[50]; //pipe per comunicare all'interno del timer
char pipe_esterna[50]; //pipe per comunicare con l'umano

pid_t figli[2]; //memorizza process-id del figlio

registro* registri[2];

lista_stringhe* tipi_figli;
lista_stringhe* stati_attesi;


int main (int argn, char** argv)
{
  tipi_figli = crea_lista();
  stati_attesi = crea_lista();

  strcpy(pipe_figlio, "");

  //registri:
  //registro che indica l'orario di attivazione
  registro attivazione;
  strcpy(attivazione.nome, "begin");
  attivazione.da_calcolare = FALSE;
  attivazione.valore.integer = 1;
  attivazione.is_intero = TRUE;

  //registro che indica l'orario di disattivazione
  registro disattivazione;
  strcpy(disattivazione.nome, "end");
  disattivazione.da_calcolare = FALSE;
  disattivazione.valore.integer = 10;
  disattivazione.is_intero = TRUE;

  int numero_registri = 2;
  registri[0] = &attivazione;
  registri[1] = &disattivazione;

  if(argn < 2) //se il numero di argomenti passati a funzione è minore di 2 --> errore
  {
    exit(130);
  }

  id = strtol(argv[1], &a, 10); //recupero l'id del componente

  if( id == 0 )
    exit(0);

  //se ricevo più argomenti da riga di Comando
  //FORMATO INPUT : nome_file id stato_timer begin end [dati_del_figlio]
  //datidelfiglio ==> stringa separata da underscore
  if(argn >= 4) //recupero registro begin
  {
    registri[0]->valore.integer = strtol(argv[3], &a, 10);
  }

  if(argn >= 5) //recupero registro end
  {
    registri[1]->valore.integer = strtol(argv[4], &a, 10);
  }

  if(argn >= 7) //recupero i dati del figlio
  {
    if((strcmp(argv[6], " ") != 0) && (strcmp(argv[6], "]") != 0) )
    {
      decodifica_controllo(argv[6]); //sostituisco '_' con ' '
      coda_stringhe* status = crea_coda_da_stringa(argv[6], " "); //creo la coda di stringhe
      genera_figlio(status); //genero il figlio
      distruggi(status);
    }
  }

  //creo le due pipe per gestire messaggi da centralina e da umano
  sprintf(pipe_interna, "%s/%d_int", (string) PERCORSO_BASE_DEFAULT, id);
  sprintf(pipe_esterna, "%s/%d_ext", (string) PERCORSO_BASE_DEFAULT, id);

  //gestisco la generazione dei processi che costituiscono il timer
  crea_processi_supporto(registri, numero_registri);

}

void decodifica_controllo(string tmp){
  //sostituisce all'interno di una stringa i caratteri '_' con gli spazi ' '
  int count = 0, j;
  for( j = 0; tmp[j] != '\0'; j++ ){
    if( tmp[j] == '[' || tmp[j] == ']'){

      if( tmp[j] == ']')
        count -= 1;

      if( count == 0 ){

        if( tmp[j-1] == '_')
          tmp[j-1] = ' ';

        if( tmp[j+1] == '_')
          tmp[j+1] = ' ';

      }
      if( tmp[j] == '[')
      count += 1;
    }
    if( count == 0 && tmp[j] == '_')
      tmp[j] = ' ';
  }

}


void crea_processi_supporto(registro* registri[], int numero_registri)
{
  //inizializza le pipe per la comunicazine e il semaforo
  crea_pipe(id, (string) PERCORSO_BASE_DEFAULT);
  pid_t pid = fork(); //genera un processo identico a se stesso (timer)
  if( pid == 0 ) //se sono il figlio (= processo appena generato)
  {
    while(1) //continua a spostare i messaggi da pipe esterna a pipe interna
    {
      char msg[200];
      read_msg(pipe_esterna, msg, 200);
      sem_wait(sem);
      send_msg(pipe_interna, msg);
      read_msg(pipe_interna, msg, 199);
      sem_post(sem);
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
        char msg[200];
        leggi_messaggio(id, (string) PERCORSO_BASE_DEFAULT, msg, 199); //legge messaggio da pipe con il padre
        sem_wait(sem);
        send_msg(pipe_interna, msg); //scrive sulla pipe interna il messaggio preso dal padre
        read_msg(pipe_interna, msg, 199); //legge dalla pipe interna
        if( strcmp(msg, "DONE") != 0 ) //se riceve "DONE" non fa nulla, qualsiasi altra cosa viene inviata sulla pipe con il padre
          manda_messaggio(id, (string) PERCORSO_BASE_DEFAULT, msg);
        sem_post(sem);
      }

    }
    else if( pid > 0 ) //se sono il padre
    {
      figli[1] = pid; //mi salvo il process-id del figlio creato dalla precedente fork
      signal(SIGINT, termina); //se riceve segnale di morte, muore
      while(1) //resta in perenne attesa sulla sua pipe interna
      {
        ascolta_e_interpreta(registri, numero_registri);
      }

    }

  }

}

void ascolta_e_interpreta(registro* registri[], int numero_registri)
{
  //leggo messaggio, elimino "\n" finale e gestisco il comando
  char messaggio[200];
  read_msg(pipe_interna, messaggio, 199); //leggo messaggio da pipe_interna
  strtok(messaggio, "\n"); //elimino lo "\n" alla fine della stringa

  //printf("[TIMER]%s\n", messaggio);
  //creo la coda di stringhe
  coda_stringhe* istruzioni = crea_coda_da_stringa(messaggio, " ");


  char comando[50];
  primo(istruzioni, comando, FALSE); //recupero il nome del comando

  if( strcmp( comando, GET_STATUS ) == 0 ) //stato del timer e del componente collegato
  {
    gestisci_STATUSGET(istruzioni);
  }
  else if( strcmp(comando, UPDATE_LABEL) == 0 ) //aggiornamento interruttori del timer e del comonente collegato
  {
    gestisci_LABELUP(istruzioni, registri, numero_registri);
  }
  else if( strcmp( comando, ID ) == 0 ) //risponde TRUE se l'id è raggiungibile attraverso il timer
  {
    gestisci_ID(istruzioni);
  }
  else if( strcmp(comando, REMOVE) == 0 ) //rimuovi timer o dispositivo collegato
  {
    gestisci_REMOVE(istruzioni);
  }
  else if( strcmp(comando, "SPAWN") == 0 ) //genera figlio
  {
    gestisci_SPAWN(istruzioni);
  }
  else if( strcmp(comando, "CONFIRM") == 0) //risponde TREU se l'id è il suo, altrimenti FALSE
  {
    gestisci_CONFIRM(istruzioni);
  }
  else //altri comandi non supportati
  {
    send_msg(pipe_interna, "DONE");
  }
  distruggi(istruzioni);

}

void gestisci_STATUSGET(coda_stringhe* istruzioni)
{
  //printf("[TIMER STATUSGET INIZIO]\n");
  char id_ric[50];
  primo(istruzioni, id_ric, FALSE); //recupero l'id
  int id_comp = strtol(id_ric, &a, 10);

  //printf("[TIMER STATUSGET]\n");
  boolean override = FALSE;

  //se l'id è il mio, restitusco il mio stato e quello del figlio
  if( id_comp == id || id_comp == ID_UNIVERSALE )
  {
    // Creo il messaggio contenente la risposta.
    char response[200];
    sprintf(response, "%s timer %d" ,GET_STATUS_RESPONSE, id);

    char msg[1024];
    char stato_figlio[1024];
    strcpy(stato_figlio, "");
    sprintf(msg, "%s %d", GET_STATUS, ID_UNIVERSALE ); //preparo messaggio da inviare al figlio
    //printf("[PRONTO PER INVIARE]\n");
    //se il figlio non esiste o ci sono problemi in lettura e scrittura
    if( strcmp(pipe_figlio, "") == 0 || send_msg(pipe_figlio, msg) == FALSE || read_msg(pipe_figlio, stato_figlio, 199) == FALSE  )
    {
      //printf("[TIMER ENTRATO]\n");
      strcpy(pipe_figlio, ""); //elimino la pipe de figlio
    }
    else //nessun problema nel comunicare con il filgio
    {
      char str[1024];
      strcpy(str, stato_figlio);
      //printf("[TIMER ELSE]");
      //calcolo se c'è stato override
      if(override == FALSE)
      {
        override = calcola_override(str + strlen(GET_STATUS_RESPONSE), tipi_figli, stati_attesi);
      }
      //printf("[TIMER OVERRIDE CALCOLATO]\n");
      //nello stato del figlio inserisco gli '_'
      int i = 0;
      for( i = 0; stato_figlio[i] != '\0'; i++ )
      {
        if( stato_figlio[i] == ' ')
        {
          stato_figlio[i] = '_';
        }
      }
      strcpy(msg, stato_figlio);
      strcpy(stato_figlio, msg+strlen(GET_STATUS_RESPONSE)+1 );

    }
    // preparo il messaggio finale con il mio stato e lo stato del figlio
    // Rispondo sulla pipe_interna
    sprintf(msg, " %s %d %d [ ", override == TRUE ? "TRUE" : "FALSE", registri[0]->valore.integer, registri[1]->valore.integer);
    strcat(response, msg);
    strcat(response, stato_figlio);
    strcat(response, " ]");
    send_msg(pipe_interna, response);
    //printf("[RISPOSTO]\n");

  }
  else //se l'id non è il mio, rimando il messaggio al figlio
  {
    if(strcmp(pipe_figlio, "") != 0) //se il figlio esiste
    {
      //chiedo al figlio se posso raggiungere id_ric attraverso lui, se si mando msg altrimenti no
      char msg[200], res[200];
      sprintf(msg, "%s %s", ID, id_ric); //messaggio che chiede al figlio se può raggiungere quell'id

      //invio messaggio al figlio
      if(send_msg(pipe_figlio, msg) == FALSE || read_msg(pipe_figlio, res, 199) == FALSE)
      {
        strcpy(pipe_figlio, ""); //se non riesco a leggere o scrivere elimino pipe del figlio
      }
      //se posso passare per il figlio, gli rimando il messaggio originale
      else if(strcmp(res, "TRUE") == 0)
      {
        //ricostruisco il messaggio originale
        char msg2[1024];
        sprintf(msg2, "%s %s", GET_STATUS , id_ric);
        char tmp[200];
        while( istruzioni->testa != NULL )
        {
          strcpy(tmp, istruzioni->testa->val);
          nodo_stringa* it = istruzioni->testa;
          istruzioni->testa = istruzioni->testa->succ;
          free(it->val);
          free(it);
          strcat(msg2, " ");
          strcat(msg2, tmp);
        }
        //invio al figlio il messaggio
        send_msg(pipe_figlio, msg2);

        //rinviare sopra il messaggio (al padre del timer)
        char res2[1024];
        read_msg(pipe_figlio, res2, 1023); //leggo lo stato del figlio
        send_msg(pipe_interna, res2); //rinvio il messaggio "sopra"
      }

    }
    else
    {
      //informo gli altri processi del timer che ho concluso
      send_msg(pipe_interna, "DONE");
    }

  }


}

void gestisci_CONFIRM(coda_stringhe* istruzioni)
{
  // Recupero l'ID e rispondo se è il mio o no.
  char id_ric[20];
  primo(istruzioni, id_ric, FALSE);
  int id_comp = strtol(id_ric, &a, 10);
  if( id_comp == id || id_comp == ID_UNIVERSALE ) //se l'id è mio
  {
    send_msg(pipe_interna, "TRUE");
  }
  else //se l'id non è mio
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
  //uccido i miei filgi (processi timer)
  kill(figli[0], SIGKILL);
  kill(figli[1], SIGKILL);

  close(file); //chiudo il file descriptor

  //mando al figlio messaggio di morte
  char msg[50];
  sprintf(msg, "%s %d", REMOVE, ID_UNIVERSALE);
  if(strcmp(pipe_figlio, "") != 0) //se figlio esiste
  {
    send_msg(pipe_figlio, msg);
  }

  // Distruggo tutte le pipe.
  ripulisci(id, (string) PERCORSO_BASE_DEFAULT);
  exit(0); //chiudi tutto

}

void gestisci_REMOVE(coda_stringhe* istruzioni)
{
  // Recupero l'ID e in caso mi termino.
  char id_ric[20];
  primo(istruzioni, id_ric, FALSE);
  int id_comp = strtol(id_ric, &a, 10);
  if( id_comp == id || id_comp == ID_UNIVERSALE ) //se l'id corrisponde al mio
  {
    termina(0);
  }
  else //chiedere id al figlio, se è il figlio a dover morire
  {
    if(strcmp(pipe_figlio, "") != 0) //se il figlio esiste
    {
      //chiedo a mio figlio se è lui che deve morire
      char msg[200];
      char res[20];
      sprintf(msg, "%s %s", "CONFIRM", id_ric);

      send_msg(pipe_figlio, msg); //invio richiesta di id al figlio
      read_msg(pipe_figlio, res, 19); //leggo risposta dal figlio

      //gli invio messaggio per rimuoverlo
      char die_msg[100];
      sprintf(die_msg, "%s %s", REMOVE, id_ric); //preparo il messaggio di rimozione per il figlio
      send_msg(pipe_figlio, die_msg);

      if(strcmp(res, "TRUE") == 0) //se mio figlio deve morire
      {
        //elimino mio figlio
        strcpy(pipe_figlio, "");
      } else {

        read_msg(pipe_figlio, res, 19);

      }

    }
    send_msg(pipe_interna, "TRUE");//informo gli altri processi che ho finito
  }
}

void gestisci_SPAWN(coda_stringhe* istruzioni)
{
  //controllo se sono io a dover generare un figlio se si genero
  char id_ric[20];
  primo(istruzioni, id_ric, FALSE);
  int id_comp = strtol(id_ric, &a, 10);
  if( id_comp == id || id_comp == ID_UNIVERSALE ) //se l'id è mio
  {
    genera_figlio(istruzioni); //genero il figlio
  }
  else //se non è mio l'id
  {
    //ricostruisco il messaggio e lo rinvio al figlio
    char msg[1024];
    sprintf(msg, "%s %s", "SPAWN", id_ric);
    char tmp[200];
    while( primo(istruzioni, tmp, FALSE) == TRUE )
    {
      strcat(msg, " ");
      strcat(msg, tmp);
    }

    send_msg(pipe_figlio, msg); //invio messaggio SPAWN al figlio
    send_msg(pipe_interna, "DONE"); //informo i processi del timer che ho finito
  }

}

void genera_figlio(coda_stringhe* status)
{
  //prendo in input una coda di strighe con i dati del filgio
  char tmp[40], percorso[50];
  primo(status, tmp, FALSE);
  sprintf(percorso, "./%s.out", tmp);
  //printf("[PERCORSO]%s\n", percorso);

  pid_t pid = fork(); //genero un processo identico a me

  if( pid == 0 )     // Se sono il figlio cambio l'immagine.
  {
    char* params[] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

    params[0] = (char*) malloc(sizeof(char)*40);
    strcpy(params[0], percorso);

    primo(status, tmp, FALSE);

    params[1] = (char*) malloc(sizeof(char)*40);
    strcpy(params[1], tmp);

    int i = 2;
    while( status -> testa !=  NULL )
    {
      nodo_stringa* it = status -> testa;
      status -> testa = status -> testa -> succ;
      strcpy(tmp, it -> val);
      free(it -> val);
      free(it);
      params[i] = (char*) malloc((strlen(tmp)+1) * sizeof(char));
      strcpy(params[i], tmp);
      i++;
    }

    //printf("HELLO IT'S ME MARIO\n");
    execv(params[0], params);

    int j = 0;
    for( j = 0; j <= i; j++ )
      free(params[j]);

  }
  else if( pid > 0 )
  {
    // Se sono il padre aggiungo la pipe del figlio appena creato.
    primo(status, tmp, FALSE);
    if( strcmp(pipe_figlio, "") != 0 ) //se il figlio esiste
    {
      char msg[100];
      sprintf(msg, "%s %d", REMOVE, ID_UNIVERSALE);
      send_msg(pipe_figlio, msg);
    }
    sprintf(pipe_figlio, "%s/%s", (string) PERCORSO_BASE_DEFAULT, tmp);

  }
  //informo i processi del timer di aver terminato
  send_msg(pipe_interna, "DONE");

}


void gestisci_LABELUP(coda_stringhe* istruzioni, registro* registri[], int numero_registri)
{
  registro* begin = registri[0];
  registro* end = registri[1];

  //recupero l'id
  char id_ric[50];
	primo(istruzioni, id_ric, FALSE);
	int id_comp = strtol(id_ric, &a, 10);
  boolean ret = FALSE;

  if( id_comp == id || id_comp == ID_UNIVERSALE ) //se l'azione è per me
  {
    char interruttore[50];
    char nuovo_valore[20];

    primo(istruzioni, interruttore, FALSE); //recupero l'interruttore da gestire
    primo(istruzioni, nuovo_valore, FALSE); //recupero il valore

    if(strcmp(interruttore, "BEGIN") == 0 && (strtol(nuovo_valore, &a, 10) >= 1)) //se devo agire sul registro begin
    {
      begin -> valore.integer = strtol(nuovo_valore, &a, 10); //imposto valore del registro
      signal(SIGALRM, gestisci_begin);//imposto alarm che attiva il timer
      alarm(begin -> valore.integer); //in secondi
      ret = TRUE;
    }
    else if(strcmp(interruttore, "END") == 0 && (strtol(nuovo_valore, &a, 10) >= 1)) //se devo agire sul registro end
    {
      end -> valore.integer = strtol(nuovo_valore, &a, 10);
      ret = TRUE;
    }

    else //se non è un mio registro
    {

      if(strcmp(pipe_figlio, "") != 0) //se il figlio esiste
      {
        //ricostruisco il  messaggio e lo invio al figlio
        char msg[1024];
        sprintf(msg, "%s %d %s %s", UPDATE_LABEL, ID_UNIVERSALE, interruttore, nuovo_valore);


        //printf("[INVIO]\n");
        send_msg(pipe_figlio, msg);
        read_msg(pipe_figlio, msg, 19);

        ret = strcmp(msg, "TRUE") == 0 ? TRUE : FALSE;

        //printf("[RICEVO]\n");

        if( ret == TRUE ){

          char comando[200];
          sprintf(comando, "%s %d", GET_STATUS, ID_UNIVERSALE); //preparo il messaggio
          send_msg(pipe_figlio, comando);

          char stato[1024];
          read_msg(pipe_figlio, stato, 1023); // leggo risposta dello stato del figlio
          aggiorna_stati(stato + strlen(GET_STATUS_RESPONSE) + 1);
        }
      }



    }

  }
  else //rinviare al figlio
  {
    //se figlio esiste gli mando messaggoi
    if(strcmp(pipe_figlio, "") != 0)
    {
      //ricostruisco il  messaggio e lo invio al figlio
      char msg[1024];
      sprintf(msg, "%s %s", UPDATE_LABEL, id_ric);
      char tmp[200];
      while( istruzioni->testa != NULL )
      {
        strcpy(tmp, istruzioni->testa->val);
        nodo_stringa* it = istruzioni->testa;
        istruzioni->testa = istruzioni->testa->succ;
        free(it->val);
        free(it);
        strcat(msg, " ");
        strcat(msg, tmp);
      }

      send_msg(pipe_figlio, msg);
      read_msg(pipe_figlio, msg, 19);
    }


  }
  send_msg(pipe_interna, "TRUE");

}

void gestisci_begin(int x)
{
  //quando il tiemr si attiva, deve azionare i dispositivo collegato
  if(strcmp(pipe_figlio, "") != 0) //se il figlio esiste
  {
    //invio messaggio di apertura e chiusura
    char msg1[200], msg2[200], msg3[200];
    sprintf(msg1, "%s %d %s %s", UPDATE_LABEL, ID_UNIVERSALE, "ACCENSIONE", "ON");
    sprintf(msg2, "%s %d %s %s", UPDATE_LABEL, ID_UNIVERSALE, "OPEN", "ON");
    sprintf(msg3, "%s %d %s %s", UPDATE_LABEL, ID_UNIVERSALE, "APERTURA", "ON");


    //leggo la risposta del filgio
    send_msg(pipe_figlio, msg1);
    read_msg(pipe_figlio, msg1, 199);
    send_msg(pipe_figlio, msg2);
    read_msg(pipe_figlio, msg2, 199);
    send_msg(pipe_figlio, msg3);
    read_msg(pipe_figlio, msg3, 199);


    //recupero lo stato del figlio
    char stato[1024];
    sprintf(msg1, "%s %d", GET_STATUS, ID_UNIVERSALE);
    send_msg(pipe_figlio, msg1);
    read_msg(pipe_figlio, stato, 1023);
    aggiorna_stati(stato + strlen(GET_STATUS_RESPONSE)+1);

    //imposto la alarm per inviare un segnale quando scade il timer
    alarm(0);
    signal(SIGALRM, gestisci_end);
    alarm(registri[1] -> valore.integer);
  }

}

void gestisci_end(int x)
{
  //quando scade il timer deve chiudere/spegnere il dispositivo collegato
  if(strcmp(pipe_figlio, "") != 0) //se figlio esiste
  {
    //preparo messaggio da inviare
    char msg1[200], msg2[200], msg3[200];
    sprintf(msg1, "%s %d %s %s", UPDATE_LABEL, ID_UNIVERSALE, "ACCENSIONE", "OFF");
    sprintf(msg2, "%s %d %s %s", UPDATE_LABEL, ID_UNIVERSALE, "CLOSE", "ON");
    sprintf(msg3, "%s %d %s %s", UPDATE_LABEL, ID_UNIVERSALE, "APERUTRA", "OFF");

    //invio messaggio al figlio
    send_msg(pipe_figlio, msg1);
    read_msg(pipe_figlio, msg1, 199);
    send_msg(pipe_figlio, msg2);
    read_msg(pipe_figlio, msg2, 199);
    send_msg(pipe_figlio, msg3);
    read_msg(pipe_figlio, msg3, 199);

    //chiedo al figlio il suo nuovo stato
    char stato[1024];
    sprintf(msg1, "%s %d", GET_STATUS, ID_UNIVERSALE);
    send_msg(pipe_figlio, msg1);
    read_msg(pipe_figlio, stato, 1023);

    //aggiorno lo stato atteso del figlio
    aggiorna_stati(stato + strlen(GET_STATUS_RESPONSE)+1);
  }
}


void gestisci_ID(coda_stringhe* istruzioni)
{
  // Recupero l'ID e rispondo se riesco a raggiungerlo
  char id_ric[20];
  primo(istruzioni, id_ric, FALSE);
  int id_comp = strtol(id_ric, &a, 10);

  //se l'id è il mio -> rispondo TRUE (= sono io)
  if( id_comp == id || id_comp == ID_UNIVERSALE )
  {
    send_msg(pipe_interna, "TRUE");
  }
  else //se non sono io devo chiedere a mio figlio se è in grado di raggiungerlo
  {
    if(strcmp(pipe_figlio, "") != 0) //se filgio esiste
    {
      char msg[200];
      char res[20];
      sprintf(msg, "%s %s", ID, id_ric);

      send_msg(pipe_figlio, msg); //invio messaggio al figlio
      read_msg(pipe_figlio, res, 19); //aspetto risposta del filgio

      send_msg(pipe_interna, res); //invio la risposta ricevuta nella pipe interna

    } else
    {
      send_msg(pipe_interna, "FALSE");
    }
  }

}

boolean calcola_override(string str, lista_stringhe* tipi_figli, lista_stringhe* confronti)
{
  //calcola se c'è stato override manuale
  boolean res = FALSE;
  char copia[1024];
  strcpy(copia, str);
  coda_stringhe* coda = crea_coda_da_stringa(str, " ");

  char tipo[20];
  primo(coda, tipo, FALSE);
  //printf("[TIPO]%s\n", tipo);
  //se è un hub o un timer devo controllare anche i filgi
  if( strcmp(tipo, "hub") == 0 || strcmp(tipo, "timer") == 0 ){

    char stato[1024];
    primo(coda, stato, FALSE); // tipo
    primo(coda, stato, FALSE); // id
    primo(coda, stato, FALSE); // stato
    primo(coda, stato, FALSE); // [ BEGIN
    if( strcmp(tipo, "timer") == 0 ){

      primo(coda, stato, FALSE);
      primo(coda, stato, FALSE); //figlio del timer.

    }
    decodifica_figli(stato);
    coda_stringhe* figli = crea_coda_da_stringa(stato, " ");
    while( figli -> testa != NULL && res == FALSE ){
      nodo_stringa* it = figli -> testa;
      strcpy(stato, it -> val);
      figli -> testa = figli -> testa -> succ;
      free(it -> val);
      free(it);
      if( strcmp(stato, "]") != 0 ){
        decodifica_controllo(stato);
        res = calcola_override(stato, tipi_figli, confronti);
      }
    }
    distruggi(figli);

  } else { //ogni altr dispositivo

    nodo_stringa* it = tipi_figli -> testa;

    char confronto[20];
    primo(coda, confronto, FALSE);
    primo(coda, confronto, FALSE);

    //printf("[TO CHECK]%s\n", confronto);
    int i = 0;
    boolean trovato = FALSE;
    while( it != NULL && trovato == FALSE ){

      if( strcmp(tipo, it -> val) == 0 )
        trovato = TRUE;
      else{

        it = it -> succ;
        i++;

      }

    }

    if( trovato == FALSE ){

      append(tipi_figli, tipo);
      append(confronti, confronto);
      res = FALSE;

    } else {

      char precedente[20];

      get(confronti, i, precedente);
      //printf("[CONFRONTO]%s\n", confronto);
      //printf("[PRECEDENTE]%s\n", precedente);
      res = strcmp(precedente, confronto) == 0 ? FALSE : TRUE;

    }
    distruggi(coda);
  }

  return res;

}

void decodifica_figli( string tmp ){
  //in una stringa, sostituisce le ',' con ' '
  int count = 0;
  int j;
  for( j = 0; tmp[j] != '\0'; j++ ){
    if( tmp[j] == '[' || tmp[j] == ']'){
      count += tmp[j] == '[' ? 1 : -1;
    }
    if( count == 0 && tmp[j] == ',')
      tmp[j] = ' ';
  }

}

void aggiorna_stati(string str){
  //aggiorna la lista degli stati attesi dei figli
  char copia[1024];
  strcpy(copia, str);

  coda_stringhe* coda = crea_coda_da_stringa(str, " ");
  char tipo[50];

  primo(coda, tipo, FALSE);
  //se è un hub o un timer deve controllare anche i figli
  if( strcmp(tipo, "hub") == 0 || strcmp(tipo, "timer") == 0 ){

    char stato[1024];
    primo(coda, stato, FALSE); // tipo
    primo(coda, stato, FALSE); // id
    primo(coda, stato, FALSE); // stato
    primo(coda, stato, FALSE); // [ BEGIN
    if( strcmp(tipo, "timer") == 0 ){

      primo(coda, stato, FALSE);
      primo(coda, stato, FALSE); //figlio del timer.

    }
    decodifica_figli(stato);
    coda_stringhe* figli = crea_coda_da_stringa(stato, " ");
    while( figli -> testa != NULL ){
      nodo_stringa* it = figli -> testa;
      strcpy(stato, it -> val);
      figli -> testa = figli -> testa -> succ;
      free(it -> val);
      free(it);
      if( strcmp(stato, "]") != 0 ){
        decodifica_controllo(stato);
        aggiorna_stati(stato);
      }
    }
    distruggi(figli);
    distruggi(coda);

  } else{


    nodo_stringa* it = tipi_figli -> testa;

    char confronto[20];
    primo(coda, confronto, FALSE);
    primo(coda, confronto, FALSE);

    int i = 0;
    boolean trovato = FALSE;
    while( it != NULL && trovato == FALSE ){

      if( strcmp(tipo, it -> val) == 0 )
        trovato = TRUE;
      else{

        it = it -> succ;
        i++;

      }

    }

    if ( trovato == TRUE ) {

      strcpy(it -> val, confronto);

    }
    distruggi(coda);

  }


}

boolean calcola_registro_intero( const registro* registro, int* res )
{
  return TRUE;
}
