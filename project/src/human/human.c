#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "strutture_dati/tipi_componente.h"
#include "strutture_dati/coda_stringhe.h"
#include "comunicazione/comunicazione.h"


//funzione che manda messaggi ai dispositivi
void leggi_e_manda_messaggi();

// funzione che restituisce l'id da un messaggio
int trova_id (string input);

int main(int argn, char** argv){
	//l'umano continua a inviare messaggi
	while (1){
		leggi_e_manda_messaggi();
	}
}


//manda un messaggio al dispositivo
void leggi_e_manda_messaggi()
{
	printf("%s\n", "Inserire l'id dispositivo e il tipo di messaggio:");
	//stampare i tipi di messaggi disponibili. Attendere in input gli id e il tipo di messaggio
	printf("Digitare help per la lista di comandi disponibili\n");

	char input_string[200];
	fgets(input_string, 199, stdin);
	strtok(input_string, "\n");


	coda_stringhe* coda = crea_coda_da_stringa(input_string, " ");
	char tipo_msg[50];
	primo(coda, tipo_msg, FALSE);
	// stampo i tipi di comandi disponibili

	if (strcmp (tipo_msg, "help") == 0){
		printf ("I comandi disponibili sono i seguenti: \n");
		printf ("- help: stampa la lista dei comandi disponibili \n");
		printf ("- switch <id> <label> <pos>, ");
		printf ("dove <id> è l'id del dispositivo a cui mandare il messaggio, <label> è l'interruttore o il registro del dispositivo a cui mandare il messaggio e <pos> è la posizione o il valore su cui si vuole impostarlo. \n");
		printf ("\n ATTENZIONE: altri tipi di comandi o comandi verso dispositivi non esistenti non sono supportati \n\n");
		printf ("\n I comandi disponibili per dispositivo sono: \n");
		printf (" fridge --> 'switch <id> APERTURA ON/OFF' per aprire/chiudere il frigo, 'switch <id> DELAY <n>' e 'switch <id> TEMPERATURE <n>' per impostare il ritardo di chiusura e la temperatura interna, infine 'fill <id> <n>' per modificare il livello di riempimento del frigo \n");
		printf (" window --> 'switch <id> OPEN ON/CLOSE ON' per aprire/chiudere la finestra \n");
		printf (" bulb --> 'switch <id> ACCENSIONE ON/OFF' per accendere/spegnere la lampadina \n");
		printf (" timer --> 'switch <id> BEGIN <n> / END <n>' per  impostare gli intervalli di tempo di accensione o apertura del dispositivo figlio e i comandi del dispositivo figlio \n");
		printf (" hub --> <interruttori dei dispositivi figli> \n");
		printf (" centralina --> switch 0 GENERALE ON/OFF per accendere/spegnere la centralina e i comandi dei dispositivi figli \n");
		printf ("- exit: chiudi");
	}
	else if( strcmp(tipo_msg, "switch") == 0 )
	{
		//se è un comando switch lo invio al dispositivo collegato
		if( coda -> n >= 3 ){
			char id[20], label[100], pos[100];
			primo(coda, id, FALSE);
			char percorso_pipe[200];
			sprintf(percorso_pipe, "%s/%s_ext", (string) PERCORSO_BASE_DEFAULT, id);
			//invio a quell'id il messaggio in input
			primo(coda, label, FALSE);
			primo(coda, pos, FALSE);
			char msg[2000];
			sprintf(msg, "%s %s %s %s", UPDATE_LABEL, id, label, pos);
			send_msg( percorso_pipe, msg );

		}
	} else if( strcmp(tipo_msg, "fill") == 0 ){ //se il comando è di tipo fill mando un messaggio SET_FILL (frigo)

		if( coda -> n >= 2 ){

			char id[20], perc[20];
			primo(coda, id, FALSE);
			primo(coda, perc, FALSE);

			char msg[2000];
			sprintf(msg, "SET_FILL %s %s", id, perc);

			char pipe[200];
			sprintf(pipe, "%s/%s_ext", (string) PERCORSO_BASE_DEFAULT, id);
			send_msg(pipe, msg);

		}

	} else if (strcmp (tipo_msg, "exit") == 0) { //se il comando è exit allora esco
		exit (0);

	} else {

		printf("Comando non valido: %s\n", tipo_msg);

	}
	distruggi(coda); //elimina la coda dei messaggi
}

boolean calcola_registro_intero( const registro* r, int* res ){
	return TRUE;
}
boolean calcola_registro_stringa( const registro* r, string output){
	return TRUE;
}
