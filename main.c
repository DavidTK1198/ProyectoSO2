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
PROYECTO #1
ESTUDIANTES: DANIEL DAVID BARRIENTOS ARAYA, DANIEL MADRIGAL
PROFESOR: JOSE CALVO SUÁREZ
REFERENCIAS:
https://www.youtube.com/watch?v=decAHMKIo_A&t=580s ---> What are command line arguments (argc and argv)?
https://www.youtube.com/watch?v=YSn8_XdGH7c --> Introduction to semaphores in C
https://www.youtube.com/watch?v=Q79uEdKNVGY ---> Practical example using semaphores 
https://www.youtube.com/watch?v=Ra6p-Bmajlw ---> Implementing a cola in C
*/
long cola[3];	  //cola que representa las sillas
int contador = 0; //contador que ayuda a validar si esta vacia o esta llena
int silla=0;
int valor=0;
pthread_mutex_t lock;//cerradura que representa la silla
sem_t semaphore_est;//semaforo de estudiante
sem_t semaphore_as;//semaforo de asistente
void enqueue(long);
int dequeue();
void* atender(void*);
void* Consultar(void*);
bool estaVacia();
bool estalleno();
long peek();
int main(int argc, char *argv[]){

     valor = strtol(argv[1], NULL, 10); //convertimos el numero ingresado a int
	pthread_t estudiantes[valor];//hilos de estudiantes
    pthread_t asistente;//hilo de asistente
	 pthread_mutex_init(&lock, NULL);//iniciamos el lock
	//Inicializamos los semaforos.
	sem_init(&semaphore_est, 0, 0);
	sem_init(&semaphore_est, 0, 1);
	//iniciamos el asistente
	pthread_create(&asistente, NULL, atender,(void*)&asistente);
    for (int i = 0; i < valor; i++){
        if (pthread_create(&estudiantes[i], NULL, &Consultar, (void*)(long)i)!= 0){
            perror("Fallo al crear el hilo");
        }
    }
	pthread_join(asistente, NULL);
    for (int i = 0; i < valor; i++)
    {
        if (pthread_join(estudiantes[i], NULL) != 0)
        {
            perror("Fallo al hacer join al hilo");
        }
    }
	sem_destroy(&semaphore_as);
	sem_destroy(&semaphore_est);
    pthread_mutex_destroy(&lock);
	return 0;
}
long peek(){
	return cola[0];
}
void enqueue(long x)
{
     if (contador == 3) {
        fprintf(stderr, "Todas las sillas estan ocupadas\n");
        return;
    }
    cola[contador] = x;
    contador++;
}

int dequeue()
{
   if (contador == 0) {
        fprintf(stderr, "Ningun estudiante esperando consulta\n");
        return -1;
    }
    int res = cola[0];
    int i;
    for (i = 0; i < contador - 1; i++) {
        cola[i] = cola[i + 1];
		
    }
    contador--;
    return res;
}
bool estaVacia()
{
    return (contador == 0);
}
bool estalleno(){
	return (contador==3);
}
//en la funcion consultar el estudiante busca ayuda del asistente
void *Consultar(void *numhilo)
{
	while (true){
		sleep(rand() % 20 + 1);
		pthread_mutex_lock(&lock); //un estudiante llego a pedir consulta,ponemos cerradura para evitar
		printf("El estudiante %ld necesita ayuda del asistente\n", (long)numhilo);
			if(!estalleno()){	
			enqueue((long)numhilo);
			if(contador==1){
				printf("Asistente escuchando peticiones de consulta.\n");
			}
			sem_post(&semaphore_as);
			pthread_mutex_unlock(&lock);
			sem_wait(&semaphore_est);
			pthread_mutex_lock(&lock);
			valor--;
			pthread_mutex_unlock(&lock);
			pthread_mutex_lock(&lock);
			if(valor==0){
				sem_post(&semaphore_as);
			}
			pthread_mutex_unlock(&lock);
			break;
		}else{
			pthread_mutex_unlock(&lock);
			printf("No hay campos disponibles el estudiante %ld vuelve más tarde. \n",(long)numhilo);
			}
	}
}


//en la funcion atender el asistente "duerme" hasta tener algún estudiante en consulta
void * atender(void* thr)
{
	while (true)
		
	{	printf("El asistente está durmiendo\n"); //Al no tener estudiantes duerme
		if(valor==0){
			printf("Se termino la consulta por hoy!!!\n");
			break;
		}
		while (true)
		{ 	sem_wait(&semaphore_as);
			// revisamos si la silla esta ocupada
			pthread_mutex_lock(&lock);
			if(estaVacia()){
				pthread_mutex_unlock(&lock);
				break;
			}
			printf("Proximo estudiante %ld\n",peek());
			long estt=dequeue();
			pthread_mutex_unlock(&lock);
			printf("El estudiante %ld resolviendo dudas con el asistente\n",estt);
			sleep(rand() % 5 + 1);
			printf("El estudiante %ld termino la consulta\n",estt);
			sem_post(&semaphore_est);
		}
	}
}


