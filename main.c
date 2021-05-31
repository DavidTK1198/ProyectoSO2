#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <semaphore.h>
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
https://www.youtube.com/watch?v=Ra6p-Bmajlw ---> Implementing a queue in C
*/
int cola[2];//cola que representa las sillas
int contador = 0;//contador que ayuda a validar si esta vacia o esta llena
void enqueue(int);
int dequeue();
sem_t semaphore;

int main(int argc, char *argv[]){  
    int valor=strtol(argv[1],NULL,10);//convertimos el numero ingresado a int
       pthread_t estudiantes[valor];
    sem_init(&semaphore, 0, 3);
    int i;
    for (i = 0; i < valor; i++) {
        int* a = malloc(sizeof(int));
        *a = i;
        if (pthread_create(&estudiantes[i], NULL, &routine, a) != 0) {
            perror("Failed to create thread");
        }
    }

    for (i = 0; i < valor; i++) {
        if (pthread_join(estudiantes[i], NULL) != 0) {
            perror("Failed to join thread");
        }
    }
    sem_destroy(&semaphore);
    return 0;
}

void* routine(void* args){
 printf("(%d) Waiting in the login queue\n", *(int*)args);
    sem_wait(&semaphore);
    printf("(%d) Logged in\n", *(int*)args);
    sleep(rand() % 5 + 1);
    printf("(%d) Logged out\n", *(int*)args);
    sem_post(&semaphore);
    free(args);
}

void enqueue(int x){
     if (contador == 3) {
        fprintf(stderr, "No hay más espacio en la cola\n");
        return;
    }
   cola[contador] = x;
    contador++;
}

int dequeue(){
 if (contador == 0) {
        fprintf(stderr, "No hay elementos en la cola\n");
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
