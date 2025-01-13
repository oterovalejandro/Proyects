#include <stdio.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>

#include <math.h>

#define LSM6DS33 0x6a


#define SWITCH1 31
#define LED1 24
#define LED2 28


float lsm6ds33_get_temp(int fd, unsigned char out_0, unsigned char out_1)
{
	short int raw_data;
	
	unsigned char data_read[2];
	float temp;
	
	// leer los 16 bits del sensor
	data_read[0] = wiringPiI2CReadReg8(fd,out_0);
	data_read[1] = wiringPiI2CReadReg8(fd,out_1);
	// concatenar los 2 bytes, y procesar de acuerdo al datasheet
	
	raw_data = (data_read[1]<<8)|(data_read[0]);
	printf("data_read[0] = %d data_read[1] = %d data=%i\n",data_read[0],data_read[1],raw_data);
	temp = (double)raw_data/16+25;
	printf("temp=%f ºC\n",temp);
	return(temp);
}


void inicializacion(){
    //Estados iniciales
    digitalWrite(LED1,0);
    digitalWrite(LED2,0);
}

int main(){
	int fd;
	
	float temp;
	wiringPiSetup();
	// siempre es bueno comprobar errores...
	fd = wiringPiI2CSetup(LSM6DS33); //componente I2C de WiringPi
	//printf("fd = %d\n", fd);
	
	//Leer temperatura

	pinMode (LED1, OUTPUT) ;
    pinMode (SWITCH1,INPUT);
    pinMode (LED2, OUTPUT) ;
	
	
	pullUpDnControl(SWITCH1,PUD_UP);
	
	inicializacion();
	
	for(;;){
	delay(1e3);
	temp = lsm6ds33_get_temp(fd,0x20,0x21);
	printf("Temperature: %f ºC\n",temp);
}
	return 0;
}
