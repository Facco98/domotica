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
	}
	else if( strcmp(tipo_msg, "switch") == 0 )
	{
		//se è un ocmando switch lo invio al dispositivo collegato
		if( coda -> n >= 3 ){
			char id[20], label[100], pos[100];
			primo(coda, id, FALSE);
			char percorso_pipe[200];
			sprintf(percorso_pipe, "%s/%s_ext", (string) PERCORSO_BASE_DEFAULT, id);
			//invio a quell'id il messaggio in input

			primo(coda, label, FALSE);
			primo(coda, pos, FALSE);
			char msg[200];
			sprintf(msg, "%s %s %s %s", UPDATE_LABEL, id, label, pos);
			send_msg( percorso_pipe, msg );

		}
	} else if( strcmp(tipo_msg, "fill") == 0 ){

		if( coda -> n >= 2 ){

			char id[20], perc[20];
			primo(coda, id, FALSE);
			primo(coda, perc, FALSE);

			char msg[200];
			sprintf(msg, "SET_FILL %s %s", id, perc);

			char pipe[200];
			sprintf(pipe, "%s/%s_ext", (string) PERCORSO_BASE_DEFAULT, id);
			send_msg(pipe, msg);

		}

	} else {

		printf("Comando non valido: %s\n", tipo_msg);

	}
	distruggi(coda);


}

boolean calcola_registro_intero( const registro* r, int* res ){
	return TRUE;
}
boolean calcola_registro_stringa( const registro* r, string output){
	return TRUE;
}
