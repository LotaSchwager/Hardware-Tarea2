#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <semaphore.h>

// Struct de la barberia
typedef struct Barberia
{
	int sillas_espera;
	int barberos;
	int sillas_barberos;
} Barberia;

// Struct del cliente
typedef struct Cliente
{
	int tiempo_llegada;
	int tiempo_espera;
	int tiempo_corte;
	int id;
} Cliente;

int cantidadClientes(char *nombreArchivo);

void lectura(Barberia *b, Cliente *c, int *error, char *nombreArchivo);
