#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "strutture_dati/tipi_componente.h"
#include "strutture_dati/coda_stringhe.h"
#include "comunicazione/comunicazione.h"


/*
HUMAN

*/
//funzione sempre eseguita dall'umano, fino a quando non muore
void leggi_e_manda_messaggi();

// funzione che restituisce l'id da un messaggio
int trova_id (string input);

//nel ciclo la funzione leggi_e_manda_messaggi() che è sempre eseguita
//main
int main(int argn, char** argv){
	while (1){
		leggi_e_manda_messaggi();
	}
}


//la funzione manda_messaggi_e_interpreta() manda un messaggio e aspetta la risposta del dispositivo
void leggi_e_manda_messaggi(){
	printf("%s\n", "Inserire l'id dispositivo e il tipo di messaggio:");
	//stampare i tipi di messaggi disponibili. Attendere in input gli id e il tipo di messaggio
	printf("Digitare help per la lista di comandi disponibili");
	//sprintf() è la funzione contraria a atoi() (quindi tipo itoa())
	// crea una stringa in cui inserire l'input inserito dall'utente
	//input_string = id_dispositivo + messaggio
	char input_string[200];
	fgets(input_string, 199, stdin);

	//creo una coda dei messaggi
	coda_stringhe* coda_messaggi = crea_coda();
	//inserisco il messaggio in input nella coda
	boolean ris = inserisci(coda_messaggi, input_string);
	//trovo l'id destinatario con la funzione trova_id, che restituisce un intero
	int idD;
	char tipo_msg[200];

	char input_string1[200];
	char input_string2 [200];

	strcpy(input_string1, input_string);
	strcpy(input_string2 input_string);

	//trovo a chi devo mandare il messaggio e che tipologia di messaggio devo mandare
	idD = trova_id(input_string1);
	tipo_msg = trova_tipo_msg(input_string2);

	if (strcmp (tipo_msg, "help") == 0){
		printf ("I comandi disponibili sono i seguenti:\n");
		printf ("         - help: stampa la lista dei comandi disponibili\n");
		printf ("         - switch <id> <label> <pos>");
		printf ("                                    dove <id> è l'id del dispositivo a cui mandare il messaggio, ");
		printf ("                                    <label> è l'interruttore o il registro del dispositivo a cui mandare il messaggio e ");
		printf ("                                    <pos> è la posizione o il valore su cui si vuole impostarlo. \n");
		printf ("\n ATTENZIONE: altri tipi di comandi o comandi verso dispositivi non esistenti non sono supportati \n");
		printf ("I comandi disponibili per dispositivo sono, in formato <label> <pos>: \n");
		printf ("fridge --> OPEN ON, CLOSE ON, DELAY <n>, TEMPERATURE <n>, SET_FILL <n> (dove n è un valore)");
		printf ("window --> OPEN ON, CLOSE ON");
		printf ("bulb   --> ACCENSIONE ON");
		printf ("timer  --> BEGIN <n>, END <n>, <interruttori dei dispositivi figli>");
		printf ("hub    --> <interruttori dei dispositivi figli>");
		printf ("centralina --> GENERALE ON, GENERALE OFF, <interruttori dei dispositivi figli>");
	} else {
	char percorso_pipe[200];
	sprintf(percorso_pipe, "%s/%d_ext", (string) PERCORSO_BASE_DEFAULT, idD);
	//invio a quell'id il messaggio in input 
	send_msg( percorso_pipe, input_string );
	}

	
}

/*La funzione trova_id prende in input la stringa e restituisce la prima stringa tra i due spazi convertita in intero
* in questo caso il numero trovato sarà l'id perché i comandi dati dagli umani sono del tipo:
* "<LABELUP> 10 ACCENSIONE ON"
*/

int trova_id (string input_string){

	string res;
	res = strtok(input_string, " ");
	res = strtok(NULL, " ");
	return atoi(res);
}

/*La funzione trova_tipo_msg restituisce il tipo di comando inserito dall'utente. E' da utilizzare in caso di comando help 
* per restituire una lista di comandi disponibili 
(magari qua possiamo mettere una stampa con comandi disponibili per dispositivi )
*/

char trova_tipo_msg(string input_string) {
	string res [200];
	res = strtok (input_string, " ");
	return res;
}

boolean calcola_registro_intero( const registro* r, int* res ){
	return TRUE;
}
boolean calcola_registro_stringa( const registro* r, string output){
	return TRUE;
}
