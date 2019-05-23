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
	char input_string_copia[200];
	strcpy(input_string_copia, input_string);
	idD = trova_id(input_string);
	//devo aggiungere un h all'inizio del messaggio
	char hmessaggio[200];
	char percorso_pipe[200];
	sprintf(percorso_pipe, "%s/%d_ext", (string) PERCORSO_BASE_DEFAULT, idD);
	//sprintf(hmessaggio, "H%s", input_string_copia);
	//invio la copia a quell'id
	send_msg( percorso_pipe, input_string_copia );
}

//TODO
//questa funzione è già implementata, basta utilizzare semplicamente la funzione manda_messaggio con il messaggio modificato (con una h all'inizio)
//la funzione manda_messaggio() manda un messaggio ai vari dispositivi. L'id del dispositivo viene inserito dall'utente
/*bool hmanda_messaggio(int id, char* tipo_msg){
	bool ret = false;
	//implementare i casi per i vari tipi di messaggio
	//una volta che ho id in int e tipo_msg in string posso fare, semplicemente, così?
	//non so base_path
	//per messaggio devo fare una funzione che converte i messaggi di tipo MESSAGGIO in HMESSAGGIO, nei dispositivi ci sarà una funzione che controlla e c'è o meno l'h
	//manda_messaggio( id, base_path, messaggio ){

	return ret;
}*/

//la funzione interpreta_messaggio() riceve la risposta dai dispositivi
// la funzione trova_id prende in input la stringa e restituisce la prima stringa tra i due spazi convertita in intero
// in questo caso il numero trovato sarà l'id perché i comandi dati dagli umani sono del tipo:
// "SWITCH 10 ACCENSIONE ON"
int trova_id (string input_string){

	string res;
	res = strtok(input_string, " ");
	res = strtok(NULL, " ");
	return atoi(res);
}

/*trova id
	//avevo iniziato a fare una funzione, poi mi sono accorta esiste la funzione strtok che lo fa al posto mio, in tempo lineare
	int id_int = -1;
	int contatore_spazi = 0;
	int i;
	char  id_d[50];
	int contatore_id = 0;
	for (i = 0; (i < 199 && contatore_spazi < 3); i++ ){
		//prende solo il primo carattere dopo lo spazio, io devo prenderlo tra i due spazi
		if (input_string[i]==" "){
			contatore_spazi ++;
			id_d[contatore_id] = input_string[i+1];
			//qua dovrebbe esserci un for
		} else {

		}

	}
	return id_int;

*/
	/*
	if(hmanda_messaggio(idD, tipoM)){
		hinterpreta_messaggio();
	}
	prendo il messaggio, lo trasformo in coda, trovo di chi è l'id, aggiungo un h a inizio e invio la copia a quell'id.
*/

boolean calcola_registro_intero( const registro* r, int* res ){
	return TRUE;
}
boolean calcola_registro_stringa( const registro* r, string output){
	return TRUE;
}
