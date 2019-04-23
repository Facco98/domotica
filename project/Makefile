# Definizione delle cartelle del progetto
SRC_FOLDER=src
OUT_FOLDER=out
COMPILED_LIBRARIES_FOLDER=$(OUT_FOLDER)/libraries

# Definizione delle librerie che servono al progetto
LIBS=$(COMPILED_LIBRARIES_FOLDER)/comunicazione.o $(COMPILED_LIBRARIES_FOLDER)/strutture_dati.o

.PHONY: help build clean libraries

help:
	@echo "Mostrare info veramente utili, come ad esempio le ricette disponibili"

build: libraries componenti umano

clean:
	rm -rf $(COMPILED_LIBRARIES_FOLDER)
	rm -rf $(OUT_FOLDER)/*
	rm -rf $(OUT_FOLDER)

run:
	@echo "Run still to be implemented"

# Generazione della cartella di output
$(OUT_FOLDER):
	@echo "Sto creando la cartella $(OUT_FOLDER)"
	@mkdir $(OUT_FOLDER)


# Regole per la generazione delle librerie.
libraries: $(COMPILED_LIBRARIES_FOLDER) $(LIBS)

$(COMPILED_LIBRARIES_FOLDER): $(OUT_FOLDER)
	@echo "Sto creando la cartella $(COMPILED_LIBRARIES_FOLDER)"
	@mkdir $(COMPILED_LIBRARIES_FOLDER)

$(COMPILED_LIBRARIES_FOLDER)/comunicazione.o: $(COMPILED_LIBRARIES_FOLDER) $(SRC_FOLDER)/comunicazione/comunicazione.c $(SRC_FOLDER)/comunicazione/comunicazione.h
	@echo "Sto compilando la dipendenza 'comunicazione'"
	gcc -std=gnu90 -c $(SRC_FOLDER)/comunicazione/comunicazione.c -o $(COMPILED_LIBRARIES_FOLDER)/comunicazione.o

$(COMPILED_LIBRARIES_FOLDER)/strutture_dati.o: $(COMPILED_LIBRARIES_FOLDER) $(SRC_FOLDER)/strutture_dati/strutture_dati.c $(SRC_FOLDER)/strutture_dati/strutture_dati.h
	@echo "Sto compilando la dipendenza 'strutture_dati'"
	gcc -std=gnu90 -c $(SRC_FOLDER)/strutture_dati/strutture_dati.c -o $(COMPILED_LIBRARIES_FOLDER)/strutture_dati.o


# Regole per la generazione dei componenti.
componenti:
	@echo "Componenti still to be implemented"


# Regole per la generazione dell'umano

umano:
	@echo "Umano still to be implemented"
