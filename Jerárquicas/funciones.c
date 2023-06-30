#include "funciones.h"
#include "lectura.h"

// Se crea un array de los structs
struct Barberia friseurladen;
struct Cliente *kunde;

// Mutex
pthread_mutex_t servicio_mutex;
pthread_mutex_t printf_mutex;

// Semaforos
sem_t wait_chair_sem;
sem_t customer_sem;
sem_t barber_chair;

// variables que se utilizaran para moverse entre barbero y cliente
int posicion = 0;
int tiempo_total = 0;
int todos_finalizado = 0;
int sillas_barbero_disponibles = 0;
int sillas_espera_disponibles = 0;
int sitio_silla_disponibles = -1;
int sitio_Barb_disponibles = 0;
int *id_hilos;

void *barbero(void *arg)
{
	int id = *(int *)arg;
	printf("Barbero %d entra a la barberia\n\n", id);

	while (todos_finalizado == 0)
	{
		sem_wait(&customer_sem);

		if (sillas_barbero_disponibles >= 1)
		{
			sem_wait(&wait_chair_sem);
			sillas_espera_disponibles++;
			sillas_barbero_disponibles--;
			sem_post(&wait_chair_sem);

			pthread_mutex_lock(&servicio_mutex);
			printf("El barbero %d esta cortando el pelo al cliente %d\n", id, posicion);
			sleep(kunde[posicion].tiempo_corte);
			sillas_barbero_disponibles++;
			pthread_mutex_unlock(&servicio_mutex);

			sem_post(&barber_ready_sem);
		}
	}

	printf("El barbero %d se ha ido a su casa\n", id);
	pthread_exit(NULL);
}

void *cliente(void *arg)
{
	int miAsiento, b;
	Cliente client = *(Cliente *)arg;

	sem_wait(&wait_chair_sem);
	posicion = client.id;
	printf("\nCliente %d entra a la barberia\n", client.id);

	if (sillas_espera_disponibles >= 1)
	{
		sillas_espera_disponibles--;
		// id_hilos[client.id] = pthread_self();
		printf("El cliente %d se ha sentado en la silla de espera || Asientos de espera restantes : %d\n", client.id, sillas_espera_disponibles);
		sem_post(&wait_chair_sem);

		sem_post(&customer_sem);
		sem_wait(&barber_ready_sem);

		printf("El cliente %d se fue con el pelo cortado\n", client.id);
	}
	else
	{
		sem_post(&wait_chair_sem);
		printf("Cliente %d encontro la barberia llena y se fue\n", client.id);
	}
	pthread_exit(NULL);
}

void controlador()
{
	// Se crea el array de clientes
	int size = cantidadClientes() - 2;
	kunde = (Cliente *)malloc(size * sizeof(Cliente));

	// Se lee el archivo y se guardan los valores
	lectura(&friseurladen, kunde);

	// Creando los hilos
	pthread_t barber_threads[friseurladen.barberos];
	pthread_t client_threads[size];

	// Dando los valores para las sillas
	sillas_barbero_disponibles = friseurladen.sillas_barberos;
	sillas_espera_disponibles = friseurladen.sillas_espera;
	id_hilos = (int *)malloc((size + 2) * sizeof(int));

	// Inicializando los mutex
	pthread_mutex_init(&servicio_mutex, NULL);
	pthread_mutex_init(&printf_mutex, NULL);

	// inicializando los semaforos de las sillas
	sem_init(&wait_chair_sem, 0, 1);
	sem_init(&customer_sem, 0, 0);
	sem_init(&barber_chair, 0, 0);

	// Inicializando el/los barbero/barberos
	printf("Se abre la barberia\n\n");

	if (friseurladen.barberos <= 0) // Si hay 0 o menos barberos no atender la barberia
	{
		return;
	}

	for (int i = 0; i < friseurladen.barberos; i++)
	{
		pthread_create(&barber_threads[i], NULL, barbero, (void *)&i);
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

	todos_finalizado = 1;
	// sem_post(&desperta_bobo);
	for (int i = 0; i < friseurladen.barberos; i++)
	{
		pthread_join(barber_threads[i], NULL);
	}

	pthread_mutex_destroy(&servicio_mutex);
	pthread_mutex_destroy(&printf_mutex);

	sem_destroy(&wait_chair_sem);
	sem_destroy(&barber_ready_sem);
	sem_destroy(&customer_sem);

	free(kunde);
	free(id_hilos);

	printf("\n\nSe cierra la barberia\n");
	exit(EXIT_SUCCESS);
}