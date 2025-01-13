/*
 * Uso: 
 * 		Cthilos arg1 arg2 ... argn 
 * 
 * El programa ejecuta n hilos cada uno de los cuales recibe como 
 * argumento un argi, y escribe en pantalla un número aleatorio
 * de veces el argumento recibido
 */

#define _REENTRANT
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>

#define MAX 10

// La macro TRAND genera un valor entero seudo-aleatorio 
// comprendido entre 1 y "limit" y lo almacena en "n". Para 
// favorecer la aleatoriedad usa como semilla el componente
// micro-segundos (usec) de la fecha-hora actual

#define TRAND(limit,n) {struct timeval t;\
                        gettimeofday(&t, (void *)NULL);\
                        (n) = rand_r((unsigned *)&t.tv_usec) % (limit)+1;}

void *dilo(void *);

int main(int argc, char *argv[]) {
    pthread_t thread_id[MAX];
    int i,*p_status; 
   
    if (argc > MAX + 1)
	fprintf(stderr, "Uso: %s arg1 arg2 ... arg%d\n", *argv, MAX),
	    exit(1);

    printf("Visualizacion\n");
    for (i = 0; i < argc - 1; ++i) {
		if (pthread_create(&thread_id[i],NULL,dilo,(void *) argv[i + 1]) > 0)
		    fprintf(stderr, "Fallo de pthread_create\n"), exit(2);
    }
    for (i = 0; i < argc - 1; ++i) {
		if (pthread_join(thread_id[i], (void **) &p_status) > 0)
		    fprintf(stderr, "Fallo de join\n"), exit(3);
    }
    printf("\nAcabado\n");
    exit(0);
}

void *dilo(void *palabra) {
    int i, numero;

    TRAND(MAX, numero);
    for (i = 0; i < numero; ++i) {
		sleep(1);
		printf("%s ", (char *) palabra);
		fflush(stdout);
    }
    printf("\n\t\t%d  veces se ha generado: %s\n", numero, (char *) palabra);
    fflush(stdout);
    return (void *) NULL;
}
