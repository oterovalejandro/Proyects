#include <stdio.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
// int wiringPiSPISetup (int channel, int speed) ; velocidad del clock
//int wiringPiSPIDataRW (int channel, unsigned char *data, int len) ; simultaneous write/read transaction over the selected SPI bus
// analogRead() ;
#include <mcp3004.h>
#define spi_channel  0
#define pinbase 100
#define canal 0
#define pin 100
#define velocidad 1000000 // 1 MHz
#define MOSI 12
#define MISO 13
#define SCLK 14
#define CEO 10

//// Declarar variables globales spi_channel, pin_base...
//// complementario: tiempo inicio bucle
int main() {
	int frec_muestreo;
	//Configura WiringPi
	wiringPiSetup();
	//pinMode(MOSI,INPUT);
		//pinMode(MISO,OUTPUT);
		//pinMode(SCLK,OUTPUT);
		//pinMode(CEO,INPUT);

	//pinMode(pinbase,INPUT);
	//pinMode(
	wiringPiSPISetup(spi_channel,50000);
	mcp3004Setup (pinbase, spi_channel);
	//int mcp3004Setup(pinbase,spi_channel);
	//int frec_muestreo;
	printf("Introduce la frecuencia con la que deseas muestrear en kHZ:\n");
	scanf("%d",&frec_muestreo);

//// Calculo de periodo de muestreo
float t_muestreo=1/(frec_muestreo*1000);
int n_muestras=300;
float t1;
float t2;
int i=0;
float datos[n_muestras];
//int t_ini = micros();
while(i<n_muestras) {
	t1=micros();
	datos[i]=analogRead(pin)*3.3/1023;
	t2=micros();

if (t_muestreo*(10^(6))<(t2-t1)){
printf("Selecciona una frecuencia más pequeña\n");

}
else {
delay(t_muestreo-(t2-t1)*(10^(-6))); //EL tiempo entre medidad y medida menos lo que tardamos en leer el pin
}
	printf("En la %d iteració la tensión fue %f y tardo %f\n",i,datos[i],t2-t1);
	
	i++;
}

printf("Tiempo de muestreo %f \n",t_muestreo);
//// Calcular inicio tiempo de lectura de pin
//// Lectura del ADC
//// Calcular fin tiempo de lectura de pin
//// Ajuste de muestreo
//// Control de muestra perdida
//// Imprimir valor de iteración, tensión, tiempo
//// Efectuar retardo calculado
return 0;
}
/// complementario: tiempo final bucle
/// complementario: cálculo de frecuencia efectiva de lectura