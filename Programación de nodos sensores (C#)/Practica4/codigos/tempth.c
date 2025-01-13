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
#include <wiringPi.h>
#include <wiringPiI2C.h>


#define MAX 10
#define LSM6DS33 0x6a


#define SWITCH1 31
#define LED1 24
#define LED2 28
// La macro TRAND genera un valor entero seudo-aleatorio 
// comprendido entre 1 y "limit" y lo almacena en "n". Para 
// favorecer la aleatoriedad usa como semilla el componente
// micro-segundos (usec) de la fecha-hora actual


void *temperature(void *);

int main() {
    int rc; 
    wiringPiSetup();
    float *temp; 
    pthread_t thread1;

    if ((rc=pthread_create(&thread1,NULL,&temperature,(void *) &temp))){
	printf("Fallo de pthread:\n");
    }
    pthread_join(thread1,NULL);
    printf("From main proces IC: %d\n",(int)getpid());
    
    exit(0);
    return 0;
}

void *temperature(void *temp)
{
    double *temp_data;
    temp_data = (double *)temp;
    
    short int raw_data;
    unsigned char data_read[2];
    int fd  = wiringPiI2CSetup(LSM6DS33); 
    
    while(1){
    data_read[0] = wiringPiI2CReadReg8(fd,0x20);
    data_read[1] = wiringPiI2CReadReg8(fd,0x21);

    raw_data = (data_read[1]<<8)|(data_read[0]);

    *temp_data = (double)raw_data/16+25;
    
    printf("temp=%f ºC\n",*temp_data);
    delay(1e4);
}
return (void *) NULL;
}
