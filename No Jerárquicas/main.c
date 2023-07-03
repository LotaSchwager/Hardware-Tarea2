/*
Integrantes:
 -Ademir Muñoz 20.158.396-9
 -Rodrigo Araos 21.154.061-3
 -Fernando Guerrero 19.540.726-6
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdbool.h>
#include "lectura.h"

//Array de los structs
struct Barberia barber;
struct Cliente *clientArray;

//Semaforos
sem_t *waitChairSem;
sem_t *customerSem;
sem_t *barberChair;
sem_t *barberSem;

int posicion = 0;
int cantBarberos;
int *barberosId;
int sillasBarberoDisponibles = 0;
int sillasEsperaDisponibles = 0;
int todosFinalizados = 0;
int tiempoTotal = 0;

int cantidadClientes(char *nombreArchivo){
    int size = 0;
    FILE *f = fopen(nombreArchivo, "r");
    if (!f){
        return -1;
    }

    for (char c = getc(f); c != EOF; c = getc(f)){
        if (c == '\n'){
            size += 1;
        }
    }

    fclose(f);
    return size;
}

void *barbero(int arg){
    int id = arg;
    printf("Barbero %d entra a la barberia\n", id);

    while (todosFinalizados == 0){
        sem_wait(customerSem);

        if (todosFinalizados != 0){
            break;
        }

        printf("El barbero %d le esta cortando el pelo al cliente %d\n", id, posicion);

        sleep(clientArray[posicion].tiempoCorte);

        printf("El barbero %d termino de cortar el pelo\n", id);
        sillasBarberoDisponibles++;
        sem_post(barberSem);
    }

    printf("El barbero %d se ha ido a su casa\n", id);
}

void *cliente(void *arg){
    int tiempo = 0;
    Cliente client = *(Cliente *)arg;

    sem_wait(waitChairSem);
    posicion = client.id;
    printf("El cliente %d entra a la barberia\n", client.id);

    if (sillasEsperaDisponibles > 0){
        sillasEsperaDisponibles--;
        printf("Cliente %d se sento en la silla de espera | sillas de espera restantes : %d\n", client.id, sillasEsperaDisponibles);
        sem_post(waitChairSem);

        sem_wait(barberChair);

        while(true){
            sleep(1);

            if (sillasBarberoDisponibles > 0){
                sillasEsperaDisponibles++;
                sillasBarberoDisponibles--;
                printf("Cliente %d se sento en la silla del barbero | sillas de barbero disponibles : %d\n", client.id, sillasBarberoDisponibles);
                sem_post(barberChair);

                sem_post(customerSem);

                sem_post(barberSem);

                printf("El cliente %d se fue con el pelo cortado\n", client.id);
                break;
            } else {
                tiempo++;

                if (tiempo >= client.tiempoEspera){
                    printf("El cliente %d encontro la barberia llena, se va\n", client.id);
                }
            }
        }
    }
}

void coordinador(){
    char nombreArchivo[31];
    scanf("%s", nombreArchivo);

    int size = cantidadClientes(nombreArchivo) - 2;
    if (size == -1){
        printf("Error al cargar el archivo\n");
        return;
    }

    clientArray = (Cliente *) malloc (size * sizeof(Cliente));

    int error = 0;
    
    lectura(&barber, clientArray, nombreArchivo, &error);
    if (error){
        printf("Error al cargar el archivo\n");
        return;
    }

    if (barber.barberos > barber.sillasBarberos){
        cantBarberos = barber.sillasBarberos;
    } else {
        cantBarberos = barber.barberos;
    }

    waitChairSem = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    customerSem = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    barberChair = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    barberSem = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    sillasBarberoDisponibles = cantBarberos;
    sillasEsperaDisponibles = barber.sillasEspera;

    barberosId = (int *) malloc (cantBarberos * sizeof(int));
    for (int i = 0; i < cantBarberos; i++){
        barberosId[i] = i;
    }

    //Inicializamos los semáforos
    sem_init(waitChairSem, 1, barber.sillasEspera);
    sem_init(barberChair, 1, cantBarberos);
    sem_init(barberSem, 1, 0);
    sem_init(customerSem, 1, 0);

    //Inicializamos ahora el o los barberos
    printf("Se abre la barberia\n\n");

    pid_t pid;

    if (barber.barberos <= 0){
        return;
    }

    for (int i = 0; i < cantBarberos; i++){
        pid = fork();
        if (pid == 0) {
            barbero(barberosId[i]);
            exit(0);
        }
    }

    for (int i = 0; i < size; i++){
        pid = fork();
        if (pid == 0) {
            cliente((void *)&clientArray[i]);
            exit(0);
        }
        tiempoTotal += clientArray[i].tiempoLlegada;
        sleep(tiempoTotal);
    }

    for (int i = 0; i < cantBarberos; i++){
        wait(NULL);
    }
    
    sem_destroy(waitChairSem);
    sem_destroy(barberChair);
    sem_destroy(barberSem);
    sem_destroy(customerSem);

    munmap(waitChairSem, sizeof(sem_t));
    munmap(barberChair, sizeof(sem_t));
    munmap(barberSem, sizeof(sem_t));
    munmap(customerSem, sizeof(sem_t));

    free(clientArray);
    free(barberosId);

    printf("\n\nSe cierra la barberia\n");
}

int main(){
    coordinador();
    return 0;
}