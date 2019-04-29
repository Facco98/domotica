# Definizione delle cartelle del progetto
SRC_FOLDER=src
OUT_FOLDER=out
COMPILED_LIBRARIES_FOLDER=$(OUT_FOLDER)/libraries

# Definizione delle librerie che servono al progetto
COMUNICAZIONE_LIB=$(COMPILED_LIBRARIES_FOLDER)/comunicazione.o

STRUTTURE_DATI_LIB=$(COMPILED_LIBRARIES_FOLDER)/tipi_base.o $(COMPILED_LIBRARIES_FOLDER)/tipi_componente.o \
$(COMPILED_LIBRARIES_FOLDER)/coda_stringhe.o

LIBS=$(STRUTTURE_DATI_LIB) $(COMUNICAZIONE_LIB)


.PHONY: help build clean libraries componenti umano

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

$(COMPILED_LIBRARIES_FOLDER)/comunicazione.o: $(COMPILED_LIBRARIES_FOLDER)/tipi_base.o $(SRC_FOLDER)/comunicazione/comunicazione.c \
$(SRC_FOLDER)/comunicazione/comunicazione.h
	@echo "Sto compilando la dipendenza 'comunicazione'"
	gcc -std=gnu90 -c $(SRC_FOLDER)/comunicazione/comunicazione.c -o $(COMPILED_LIBRARIES_FOLDER)/comunicazione.o -I $(SRC_FOLDER)

$(COMPILED_LIBRARIES_FOLDER)/coda_stringhe.o: $(COMPILED_LIBRARIES_FOLDER)/tipi_base.o $(SRC_FOLDER)/strutture_dati/coda_stringhe.c \
$(SRC_FOLDER)/strutture_dati/coda_stringhe.h
	@echo "Sto compilando la dipendenza 'coda_stringhe'"
	gcc -std=gnu90 -c $(SRC_FOLDER)/strutture_dati/coda_stringhe.c -o $(COMPILED_LIBRARIES_FOLDER)/coda_stringhe.o -I $(SRC_FOLDER)

$(COMPILED_LIBRARIES_FOLDER)/tipi_base.o: $(SRC_FOLDER)/strutture_dati/tipi_base.h $(SRC_FOLDER)/strutture_dati/tipi_base.c
	@echo "Sto compilando la dipendenza 'tipi_base'"
	gcc -std=gnu90 -c $(SRC_FOLDER)/strutture_dati/tipi_base.c -o $(COMPILED_LIBRARIES_FOLDER)/tipi_base.o -I $(SRC_FOLDER)

$(COMPILED_LIBRARIES_FOLDER)/tipi_componente.o: $(SRC_FOLDER)/strutture_dati/tipi_componente.h $(SRC_FOLDER)/strutture_dati/tipi_componente.c
	@echo "Sto compilando la dipendenza 'tipi_componente'"
	gcc -std=gnu90 -c $(SRC_FOLDER)/strutture_dati/tipi_componente.c -o $(COMPILED_LIBRARIES_FOLDER)/tipi_componente.o -I $(SRC_FOLDER)

# Regole per la generazione dei componenti.
componenti:
	@echo "Componenti still to be implemented"


# Regole per la generazione dell'umano

umano:
	@echo "Umano still to be implemented"
