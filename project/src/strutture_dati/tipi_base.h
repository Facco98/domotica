#ifndef  PROGETTODOMOTICA_TIPI_BASE_H
#define  PROGETTODOMOTICA_TIPI_BASE_H

#include <string.h>
/*
* Definizione del tipo di dato booleano.
*/
enum _bool {FALSE, TRUE};
typedef enum _bool boolean;

/*
* Definizione del tipo string, per comodità.
*/
typedef char* string;
boolean prefix(const string pre, const string str);

#endif
