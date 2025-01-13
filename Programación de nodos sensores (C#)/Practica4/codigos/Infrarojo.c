////CODIGO APARTADO 2.1////

#include <stdio.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <mcp3004.h>
#define spi_channel  0
#define pinbase 100
#define velocidad 5e4// 1 MHz

#define MAX_ROWS 70

float getDistance(float measure, float *x, float *voltage){
	float dist;

	int n = 0;
	
	while(n<72){
		if(measure<*voltage){
			x++;
			voltage++;
		}
		n++;
	}
	float interval = *voltage-*(voltage-1);
	float frac = (measure-*(voltage-1))/interval;
	//printf("%f\n",interval);
	dist = *(x-1) + frac*(*x-*(x-1));
	return dist;
}

int main() {
	int frec_muestreo;
	
	wiringPiSetup();

	wiringPiSPISetup(spi_channel,velocidad); //Configuramos canal de comunicación y velocidad
	mcp3004Setup (pinbase, spi_channel);
	
	printf("Introduce la frecuencia con la que deseas muestrear en kHZ:\n");
	scanf("%d",&frec_muestreo);

//// Calculo de periodo de muestreo
float t_muestreo=1e3/(frec_muestreo); //Pasamos el tiempo de muestreo a microsegundos para compararlo con los de medida del ADC
float measure[2];
float alpha = 0.9;
float t1,t2;
float dist;

 FILE *file;

    float x[MAX_ROWS+1];
    float voltage[MAX_ROWS+1];
    int fila = 0;

    file = fopen("calibracion.csv","r");
    
    if (file == NULL) {
        printf("No se pudo abrir el archivo\n");
        return 1;
    }

    // Leer el archivo CSV
    while (fila < MAX_ROWS) {
        fscanf(file,"%f;%f",&x[fila],&voltage[fila]);
        fila++;
    }

    fclose(file);

    // Imprimir los datos almacenados en los arrays
    printf("Número de filas leídas: %d\n", fila);
    printf("Datos leídos desde el archivo:\n");
    for (int i = 0; i < MAX_ROWS; i++) {
        printf("%.2f, %.2f\n", x[i], voltage[i]);
    }
    
while(1) {
	t1 = millis();
	
	measure[0] = measure[1];
	measure[1] = analogRead(pinbase)*3.3/1023; //Paso a voltios 
	measure[1] = alpha*measure[1]+(1-alpha)*measure[0];
	
	dist = getDistance(measure[1],&x[0],&voltage[0]);
	
	printf("distancia = %f cm \n",dist);
	t2 = millis();
	
	delay(t_muestreo-(t2-t1)); //EL tiempo entre medida y medida menos lo que tardamos en leer el pin, el retardo.

	}

return 0;
}
