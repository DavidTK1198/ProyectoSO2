#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdbool.h>
/*
UNIVERSIDAD NACIONAL DE COSTA RICA
SISTEMAS OPERATIVOS - EIF-212
PROYECTO #2
ESTUDIANTES: DANIEL DAVID BARRIENTOS ARAYA, DANIEL MADRIGAL Vasquez
PROFESOR: JOSE CALVO SUÁREZ
REFERENCIAS:
https://www.youtube.com/watch?v=decAHMKIo_A&t=580s ---> What are command line arguments (argc and argv)?
https://www.youtube.com/watch?v=YSn8_XdGH7c --> Introduction to semaphores in C
https://www.youtube.com/watch?v=Q79uEdKNVGY ---> Practical example using semaphores 
https://www.youtube.com/watch?v=Ra6p-Bmajlw ---> Implementing a cola in C
*/
long cola[3];	  //cola que representa las sillas
int contador = 0; //contador que ayuda a validar si esta vacia o esta llena
int valor = 0;//variable global para ver la cantidad de estudiantes
pthread_mutex_t lock; //cerradura que representa la silla
sem_t semaphore_est;  //semaforo de estudiante
sem_t semaphore_as;	  //semaforo de asistente
void enqueue(long);//metodo de encolar
int dequeue();//metodo descolar
void *atender(void *);//metoto donde el asistente "atiende dudas"
void *Consultar(void *);//metodo donde los estudiantes esperan al asistente
bool estaVacia();//revisa si la cola esta vacia
bool estalleno();//revisa si la cola esta llena
long peek();//muestra el proximo elemento que sale de la cola
int main(int argc, char *argv[])
{

	valor = strtol(argv[1], NULL, 10); //convertimos el numero ingresado a int
	pthread_t estudiantes[valor];	   //hilos de estudiantes
	pthread_t asistente;			   //hilo de asistente
	pthread_mutex_init(&lock, NULL);   //iniciamos el lock
	//Inicializamos los semaforos.
	sem_init(&semaphore_est, 0, 0);
	sem_init(&semaphore_est, 0, 1);
	//Creamos el asistente
	pthread_create(&asistente, NULL, atender, (void *)&asistente);
	//creamos los hilos de estudiante
	for (int i = 0; i < valor; i++)
	{
		if (pthread_create(&estudiantes[i], NULL, &Consultar, (void *)(long)i) != 0)
		{
			perror("Fallo al crear el hilo");
		}
	}
	pthread_join(asistente, NULL);//hacemos que main espere el hilo asistente
	for (int i = 0; i < valor; i++)
	{	//hacemos que main espere los hilos de estudiantes
		if (pthread_join(estudiantes[i], NULL) != 0)
		{
			perror("Fallo al hacer join al hilo");
		}
	}
	sem_destroy(&semaphore_as);//destruimos el semaforo del asistente
	sem_destroy(&semaphore_est);//destruimos el semaforo de estudiante
	pthread_mutex_destroy(&lock);//destruimos la cerradura
	return 0;
}
long peek()
{
	return cola[0];//muestra el proximo elemento en salir de la cola
}
void enqueue(long x)	//inserta un elemento en la cola
{
	if (contador == 3)//como son 3 sillas el tam del vector es 3
	{
		fprintf(stderr, "Todas las sillas estan ocupadas\n");
		return;
	}
	cola[contador] = x;
	contador++;
}

int dequeue()
{
	if (contador == 0)//si contador es 0 no hay estudiantes en la cola
	{
		fprintf(stderr, "Ningun estudiante esperando consulta\n");
		return -1;
	}
	int res = cola[0];
	int i;
	for (i = 0; i < contador - 1; i++)//realizmos un corrimiento 
	{
		cola[i] = cola[i + 1];
	}
	contador--;
	return res;
}
bool estaVacia()
{//revisamos si esta vacia usando el contador
	return (contador == 0);
}
bool estalleno()
{	//revisamos si esta llena usando el contador
	return (contador == 3);
}
//en la funcion consultar el estudiante busca ayuda del asistente
void *Consultar(void *numhilo)
{
	while (true)//ciclo infinito hasta que el estudiante sea atendido por el asistente
	{
		sleep(rand() % 20 + 1);//simula que el estudiante esta programando
		pthread_mutex_lock(&lock); //un estudiante llego a pedir consulta,ponemos cerradura para evitar
		printf("El estudiante %ld necesita ayuda del asistente\n", (long)numhilo);
		if (!estalleno())//revisamos si la cola no esta llena para ingresar
		{
			enqueue((long)numhilo);//insertamos a la cola
			if (contador == 1)//si contador 1 despertamos al asistente 
			{
				printf("Asistente escuchando peticiones de consulta.\n");
			}
			sem_post(&semaphore_as);//enviamos la señal para despertar al asistente
			pthread_mutex_unlock(&lock);//quitamos la cerradura zona critica
			sem_wait(&semaphore_est);//hacemos que el semaforo de estudiante espere
			pthread_mutex_lock(&lock);//ponemos cerradura a la zona critica
			valor--;
			pthread_mutex_unlock(&lock);//quitamos la cerradura zona critica
			pthread_mutex_lock(&lock);//ponemos cerradura a la zona critica
			if (valor == 0)
			{
				sem_post(&semaphore_as);//si valor es 0 significa que ya pasaron todos los estudiantes
			}							//el asistente vuelve dormir
			pthread_mutex_unlock(&lock);//quitamos la cerradura zona critica
			break;
		}
		else
		{
			pthread_mutex_unlock(&lock);//quitamos la cerradura zona critica
			printf("No hay campos disponibles el estudiante %ld vuelve más tarde. \n", (long)numhilo);
		}
	}
}

//en la funcion atender el asistente "duerme" hasta tener algún estudiante en consulta
void *atender(void *thr)
{
	while (true)//ciclo infinito hasta que todos los estudiantes sean atendidos

	{
		printf("El asistente está durmiendo\n"); //Al no tener estudiantes duerme
		if (valor == 0)
		{
			printf("Se termino la consulta por hoy!!!\n");
			break;
		}
		while (true)//ciclo infinito de los estudiantes que estan esperando en la cola
		{//con este evitamos que el asistente se vuelva a dormir
			sem_wait(&semaphore_as);
			// revisamos si la silla esta ocupada
			//ponemos la cerradura zona critica
			pthread_mutex_lock(&lock);
			if (estaVacia())
			{	//quitamos la cerradura zona critica
				pthread_mutex_unlock(&lock);
				break;
			}
			printf("Proximo estudiante %ld\n", peek());//revismos el proximo estudiante en la cola
			long estt = dequeue();//sacamos de la cola
			pthread_mutex_unlock(&lock);//quitamos la cerradura zona critica
			printf("El estudiante %ld resolviendo dudas con el asistente\n", estt);
			sleep(rand() % 5 + 1);//simula que el estudiante esta siendo atendido por el asistente
			printf("El estudiante %ld termino la consulta\n", estt);
			sem_post(&semaphore_est);//mandamos la señal para el siguiente estudiatne
		}
	}
}
