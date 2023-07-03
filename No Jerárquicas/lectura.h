#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

//Struct de la barbería
typedef struct Barberia{
    int sillasEspera;
    int barberos;
    int sillasBarberos;
} Barberia;

//Struct del cliente
typedef struct Cliente{
    bool waiting;
    int tiempoLlegada;
    int tiempoEspera;
    int tiempoCorte;
    int id;
} Cliente;

void lectura(Barberia *b, Cliente *c, char *nombreArchivo, int *error);