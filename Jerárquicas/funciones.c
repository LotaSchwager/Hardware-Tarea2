#include "funciones.h"
#include "lectura.h"

// Se crea un array de los structs
struct Barberia friseurladen;
struct Cliente *kunde;

// Semaforos
sem_t wait_chair_sem;
sem_t customer_sem;
sem_t barber_chair;
sem_t barber;

// variables que se utilizaran para moverse entre barbero y cliente
int posicion = 0;
int tiempo_total = 0;
int todos_finalizado = 0;
int sillas_barbero_disponibles = 0;
int sillas_espera_disponibles = 0;
int cant_barberos;
int *barberos_id;

// Funcion del barbero
void *barbero(void *arg)
{
	int id = *(int *)arg;
	// Entra el barbero
	printf("Barbero %d entra a la barberia\n", id);

	// Bucle infinito haste que todos los clientes hayan sido atendidos
	while (todos_finalizado == 0)
	{
		// Se espera que el cliente este listo
		sem_wait(&customer_sem);

		if (todos_finalizado != 0)
		{
			break;
		}

		// Se sienta el cliente en la silla
		printf("El barbero %d le esta cortando el pelo al cliente %d\n", id, posicion);

		// Se corta el pelo con el tiempo de cada cliente
		sleep(kunde[posicion].tiempo_corte);

		// Se avisa que el barbero esta listo para volver a atender
		printf("El barbero %d termino de cortar el pelo\n", id);
		sillas_barbero_disponibles++;
		sem_post(&barber);
	}

	printf("El barbero %d se ha ido a su casa\n", id);
	pthread_exit(NULL);
}

// Función del cliente
void *cliente(void *arg)
{
	int tiempo = 0;
	Cliente client = *(Cliente *)arg;

	// Se entra a la primera seccion critica cuando el usuario entra a la barberia
	sem_wait(&wait_chair_sem);
	posicion = client.id;
	printf("El cliente %d entra a la barberia\n", client.id);

	// Se pregunta si hay sillas de espera disponibles
	if (sillas_espera_disponibles > 0)
	{
		sillas_espera_disponibles--;
		printf("Cliente %d se sento en la silla de espera | sillas de espera restantes : %d\n", client.id, sillas_espera_disponibles);
		sem_post(&wait_chair_sem);

		// Segunda seccion critica cuando el usuario quiere subir a la silla para cortarse el pelo
		sem_wait(&barber_chair);
		while (true)
		{
			// Preguntara cada segundo
			sleep(1);

			// El cliente estara en un bucle preguntando si hay una silla disponible
			if (sillas_barbero_disponibles > 0)
			{
				sillas_espera_disponibles++;
				sillas_barbero_disponibles--;
				printf("Cliente %d se sento en la silla del barbero | sillas de barbero disponibles : %d\n", client.id, sillas_barbero_disponibles);
				sem_post(&barber_chair);

				// Se le dice al barbero que esta listo el cliente para ser atendido
				sem_post(&customer_sem);

				// Se espera a que el barbero termine de atender
				sem_wait(&barber);

				// Se va de la barberia con el pelo cortado
				printf("El cliente %d se fue con el pelo cortado\n", client.id);
				break;
			}
			else
			{
				tiempo++;

				// Si el tiempo de espera es igual al tiempo maximo que puede esperar se va de la barberia
				if (tiempo >= client.tiempo_espera)
				{
					printf("El cliente %d espero mucho en el asiento asique se fue\n", client.id);
					break;
				}
			}
		}
	}
	else
	{
		// Si no hay sillas de espera se va de la barberia
		sem_post(&wait_chair_sem);
		printf("El cliente %d encontro la barberia llena, se va\n", client.id);
	}

	pthread_exit(NULL);
}

void controlador()
{
	// Se espera por el nombre del archivo
	char nombreArchivo[31];
	scanf("%s", nombreArchivo);

	// Se crea el array de clientes
	int size = cantidadClientes(nombreArchivo) - 2;
	if (size == -1)
	{
		printf("Error al cargar el archivo\n");
		return;
	}

	kunde = (Cliente *)malloc(size * sizeof(Cliente));

	// Se lee el archivo y se guardan los valores
	int error = 0;
	lectura(&friseurladen, kunde, &error, nombreArchivo);
	if (error)
	{
		printf("Error al cargar el archivo\n");
		return;
	}

	// Creando los hilos
	if (friseurladen.barberos > friseurladen.sillas_barberos)
	{
		cant_barberos = friseurladen.sillas_barberos;
	}
	else
	{
		cant_barberos = friseurladen.barberos;
	}

	pthread_t barber_threads[cant_barberos];
	pthread_t client_threads[size];

	// Dando los valores para las sillas
	sillas_barbero_disponibles = cant_barberos;
	sillas_espera_disponibles = friseurladen.sillas_espera;

	// Rellenando los id de los barberos
	barberos_id = (int *)malloc(cant_barberos * sizeof(int));
	for (int i = 0; i < cant_barberos; i++)
	{
		barberos_id[i] = i;
	}

	// inicializando los semaforos de las sillas
	sem_init(&wait_chair_sem, 0, friseurladen.sillas_espera);
	sem_init(&barber_chair, 0, cant_barberos);
	sem_init(&barber, 0, 0);
	sem_init(&customer_sem, 0, 0);

	// Inicializando el/los barbero/barberos
	printf("Se abre la barberia\n\n");

	if (friseurladen.barberos <= 0) // Si hay 0 o menos barberos no atender la barberia
	{
		return;
	}

	for (int i = 0; i < cant_barberos; i++)
	{
		pthread_create(&barber_threads[i], NULL, barbero, (void *)&barberos_id[i]);
	}

	// Inicializando los clientes
	for (int i = 0; i < size; i++)
	{
		// Tiempo que tarda en llegar
		tiempo_total += kunde[i].tiempo_llegada;
		sleep(tiempo_total);

		// Entra cliente
		pthread_create(&client_threads[i], NULL, cliente, (void *)&kunde[i]);
	}

	// Una vez terminado se destruyen los hilos, mutex y semaforos
	for (int i = 0; i < size; i++)
	{
		pthread_join(client_threads[i], NULL);
	}

	// Se indica que se ha atendido a todos los clientes
	todos_finalizado = 1;

	// Se activa el semaforo para que los barberos salgan del bucle y terminen su ejecucion
	for (int i = 0; i < cant_barberos; i++)
	{
		sem_post(&customer_sem);
	}

	// Los barberos salen de la barberia
	for (int i = 0; i < cant_barberos; i++)
	{
		// sem_post(&customer_sem);
		pthread_join(barber_threads[i], NULL);
	}

	// se destruyen los semaforos
	sem_destroy(&wait_chair_sem);
	sem_destroy(&barber_chair);
	sem_destroy(&customer_sem);
	sem_destroy(&barber);

	// Se libera memoria de las variables que ocupan malloc
	free(kunde);
	free(barberos_id);

	printf("\n\nSe cierra la barberia\n");
	exit(EXIT_SUCCESS);
}