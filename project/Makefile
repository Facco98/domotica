# Definizione delle cartelle del progetto
#cartella dei sorgenti
SRC_FOLDER=src
#cartella dei file *.out
OUT_FOLDER=out
#cartella delle librerie compilate
COMPILED_LIBRARIES_FOLDER=$(OUT_FOLDER)/libraries
#cartella dell'umano compilato
COMPILED_UMANO_FOLDER=$(OUT_FOLDER)/umano

# Definizione delle librerie che servono al progetto
COMUNICAZIONE_LIB=$(COMPILED_LIBRARIES_FOLDER)/comunicazione.o

STRUTTURE_DATI_LIB=$(COMPILED_LIBRARIES_FOLDER)/tipi_base.o $(COMPILED_LIBRARIES_FOLDER)/tipi_componente.o \
$(COMPILED_LIBRARIES_FOLDER)/coda_stringhe.o $(COMPILED_LIBRARIES_FOLDER)/lista_stringhe.o

LIBS=$(STRUTTURE_DATI_LIB) $(COMUNICAZIONE_LIB)



.PHONY: help build clean libraries componenti umano run rum spiegazione_processi_pipe

help:
	@echo "Ricette disponibili"
	@echo "                   - help: per mostrare questa lista (comandi disponibili)"
	@echo "                   - clean: rimuove file temporanei e riporta allo stato iniziale"
	@echo "                   - build: compila il progetto con il tool gcc e il flag -std=gnu90"
	@echo "                   - run: esegue il progetto"
		#sono da togliere le easter egg nell'help???
	@echo "                   - easter eggs: ricetta spiegazione_processi_pipe (e rum) "
		#ci sono altre ricette??
	@echo "[Mostrare altre ricette disponibili]"

build: libraries componenti umano

clean:
	rm -rf $(COMPILED_LIBRARIES_FOLDER)
	rm -rf $(OUT_FOLDER)/*
	rm -rf $(OUT_FOLDER)
	rm -rf $(COMPILED_UMANO_FOLDER)

run:
	@echo "Running ... "
	cd $(OUT_FOLDER) && ./centralina.out


# Generazione della cartella di output
$(OUT_FOLDER):

	@echo "Sto creando la cartella $(OUT_FOLDER)"
	@mkdir $(OUT_FOLDER)


# Regole per la generazione delle librerie.
libraries: $(COMPILED_LIBRARIES_FOLDER) $(LIBS)

$(COMPILED_LIBRARIES_FOLDER): $(OUT_FOLDER)
	@echo "Sto creando la cartella $(COMPILED_LIBRARIES_FOLDER)"
	@mkdir $(COMPILED_LIBRARIES_FOLDER)

#Generazione della libreria comunicazione
$(COMPILED_LIBRARIES_FOLDER)/comunicazione.o: $(COMPILED_LIBRARIES_FOLDER)/tipi_base.o $(SRC_FOLDER)/comunicazione/comunicazione.c \
$(SRC_FOLDER)/comunicazione/comunicazione.h
	@echo "Sto compilando la dipendenza 'comunicazione'"
	gcc -std=gnu90 -c $(SRC_FOLDER)/comunicazione/comunicazione.c -o $(COMPILED_LIBRARIES_FOLDER)/comunicazione.o -I $(SRC_FOLDER)

#Generazione della libreria lista_stringhe
$(COMPILED_LIBRARIES_FOLDER)/lista_stringhe.o: $(COMPILED_LIBRARIES_FOLDER)/tipi_base.o $(SRC_FOLDER)/strutture_dati/lista_stringhe.c \
$(SRC_FOLDER)/strutture_dati/lista_stringhe.h
	@echo "Sto compilando la dipendenza 'lista_stringhe'"
	gcc -std=gnu90 -c $(SRC_FOLDER)/strutture_dati/lista_stringhe.c -o $(COMPILED_LIBRARIES_FOLDER)/lista_stringhe.o -I $(SRC_FOLDER)

#Generazione della libreria coda_stringhe
$(COMPILED_LIBRARIES_FOLDER)/coda_stringhe.o: $(COMPILED_LIBRARIES_FOLDER)/lista_stringhe.o $(SRC_FOLDER)/strutture_dati/coda_stringhe.c \
$(SRC_FOLDER)/strutture_dati/coda_stringhe.h
	@echo "Sto compilando la dipendenza 'coda_stringhe'"
	gcc -std=gnu90 -c $(SRC_FOLDER)/strutture_dati/coda_stringhe.c -o $(COMPILED_LIBRARIES_FOLDER)/coda_stringhe.o -I $(SRC_FOLDER)

#Generazione della libreria tipi_base
$(COMPILED_LIBRARIES_FOLDER)/tipi_base.o: $(SRC_FOLDER)/strutture_dati/tipi_base.h $(SRC_FOLDER)/strutture_dati/tipi_base.c
	@echo "Sto compilando la dipendenza 'tipi_base'"
	gcc -std=gnu90 -c $(SRC_FOLDER)/strutture_dati/tipi_base.c -o $(COMPILED_LIBRARIES_FOLDER)/tipi_base.o -I $(SRC_FOLDER)

#Generazione della libreria tipi_componente
$(COMPILED_LIBRARIES_FOLDER)/tipi_componente.o: $(SRC_FOLDER)/strutture_dati/tipi_componente.h $(SRC_FOLDER)/strutture_dati/tipi_componente.c
	@echo "Sto compilando la dipendenza 'tipi_componente'"
	gcc -std=gnu90 -c $(SRC_FOLDER)/strutture_dati/tipi_componente.c -o $(COMPILED_LIBRARIES_FOLDER)/tipi_componente.o -I $(SRC_FOLDER)



# Regole per la generazione dei componenti.
componenti:
	@echo "Generazione delle componenti"
	
	#Generazione della centralina
	@echo "Sto creando le dipendenze della componente 'centralina'"
	$(OUT_FOLDER)/centralina.o: $(SRC_FOLDER)/centralina.c  $(COMPILED_LIBRARIES_FOLDER)/comunicazione.o $(COMPILED_LIBRARIES_FOLDER)/tipi_componente.o \
	$(COMPILED_LIBRARIES_FOLDER)/coda_stringhe.o $(COMPILED_LIBRARIES_FOLDER)/lista_stringhe.o
	@echo "Sto compilando la componente 'centralina'"
	gcc -std=gnu90 -c  $(SRC_FOLDER)/centralina.c -o $(OUT_FOLDER)/centralina.o -I $(SRC_FOLDER)

	#Generazione di hub
	@echo "Sto creando le dipendenze della componente'hub'"
	$(OUT_FOLDER)/hub.o: $(SRC_FOLDER)/componenti/hub.c $(COMPILED_LIBRARIES_FOLDER)/tipi_componente.o $(COMPILED_LIBRARIES_FOLDER)/coda_stringhe.o \
	$(COMPILED_LIBRARIES_FOLDER)/comunicazione.o
	@echo "Sto compilando la componente 'hub'"
	gcc -std=gnu90 -c  $(SRC_FOLDER)/componenti/hub.c -o $(OUT_FOLDER)/hub.o -I $(SRC_FOLDER)

	#Generazione di timer
	@echo "Sto creando le dipendenze della componente 'timer'"
	$(OUT_FOLDER)/timer.o: $(SRC_FOLDER)/componenti/timer.c $(COMPILED_LIBRARIES_FOLDER)/tipi_componente.o $(COMPILED_LIBRARIES_FOLDER)/coda_stringhe.o \
	$(COMPILED_LIBRARIES_FOLDER)/comunicazione.o
	@echo "Sto compilando la componente 'timer'"
	gcc -std=gnu90 -c  $(SRC_FOLDER)/componenti/timer.c -o $(OUT_FOLDER)/timer.o -I $(SRC_FOLDER)

	#Generazione della lampadina
	@echo "Sto creando le dipendenze della componente 'lampadina'"
	$(OUT_FOLDER)/bulb.o: $(SRC_FOLDER)/componenti/lampadina.c $(COMPILED_LIBRARIES_FOLDER)/tipi_componente.o $(COMPILED_LIBRARIES_FOLDER)/coda_stringhe.o \
	$(COMPILED_LIBRARIES_FOLDER)/comunicazione.o
	@echo "Sto compilando la componente 'lampadina'"
	gcc -std=gnu90 -c  $(SRC_FOLDER)/componenti/lampadina.c -o $(OUT_FOLDER)/bulb.o -I $(SRC_FOLDER)

	#Generazione del frigo
	@echo "Sto creando le dipendenze della componente 'frigo'"
	$(OUT_FOLDER)/fridge.o: $(SRC_FOLDER)/componenti/frigo.c $(COMPILED_LIBRARIES_FOLDER)/tipi_componente.o $(COMPILED_LIBRARIES_FOLDER)/coda_stringhe.o \
	$(COMPILED_LIBRARIES_FOLDER)/comunicazione.o
	@echo "Sto compilando la componente 'frigo'"
	gcc -std=gnu90 -c  $(SRC_FOLDER)/componenti/frigo.c -o $(OUT_FOLDER)/fridge.o -I $(SRC_FOLDER)

	#Generazione della finestra
	@echo "Sto creando le dipendenze della componente 'finestra'"
	$(OUT_FOLDER)/window.o: $(SRC_FOLDER)/componenti/finestra.c $(COMPILED_LIBRARIES_FOLDER)/tipi_componente.o $(COMPILED_LIBRARIES_FOLDER)/coda_stringhe.o \
	$(COMPILED_LIBRARIES_FOLDER)/comunicazione.o
	@echo "Sto compilando la componente 'finestra'"
	gcc -std=gnu90 -c  $(SRC_FOLDER)/componenti/finestra.c -o $(OUT_FOLDER)/window.o -I $(SRC_FOLDER)


# Regole per la generazione dell'umano

umano:
	@echo "Generazione di 'umano'"

	$(COMPILED_UMANO_FOLDER) $(LIBS)


	$(COMPILED_UMANO_FOLDER): $(OUT_FOLDER)
	@echo "Sto creando la cartella $(COMPILED_UMANO_FOLDER)"
	@mkdir $(COMPILED_UMANO_FOLDER)

	@echo "Sto creando le dipendenze per il file 'umano'"
	$(COMPILED_UMANO_FOLDER)/human.o: $(SRC_FOLDER)/human.c $(COMPILED_LIBRARIES_FOLDER)/tipi_componente.o $(COMPILED_LIBRARIES_FOLDER)/coda_stringhe.o \
	$(COMPILED_UMANO_FOLDER)/comunicazione.o 
	@echo "Sto compilando il file 'umano'"
	gcc -std=gnu90 -c  $(SRC_FOLDER)/human.c -o $(COMPILED_UMANO_FOLDER)/human.o -I $(SRC_FOLDER)

#easter egg rum: citazione a Jack Sparrow
rum: 
	@echo "Perché hai bruciato il RUUUUUUUMMM???!!?"
	@echo "WHY IS THE RUUM GOOOONEEE???!!?"

#easter egg disegno: disegno/spiegazione dei processi e delle pipe di un dispositivo
spiegazione_processi_pipe:
	@echo " pipe_interna  <---> (1) processo dispositivo                                                            "
	@echo " pipe_interna  <---- (2) processo per l'interazione esterna        <----   pipe_esterna   <---   umano   "
	@echo " pipe_interna  <---> (3) processo per l'interazione con il padre   <--->   pipe <id>      <--->  padre   "


#--------------------------------------to do list: makefile edition-------------------------------------------
#IMPLEMENTATO, DA CONTROLLARE
#- controllare la correttezza di umano
#- controllare la correttezza di componenti
#- controllare la correttezza di clean (modificato) 
#- controllare la correttezza delle cartelle nuove aggiunte
#- controllare run

#DA IMPLEMENTARE (*)
#- eliminare gli echo inutili in questo file
#- eliminare i commenti inutili in questo file
#- mostrare altre ricette disponibili in help 
#- commentare diffusamente

#DA IMPLEMENTARE (**)

#- (creare altre ricette ?)