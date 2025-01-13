#include <wiringPi.h>
#include <stdio.h>
#include <wiringPiSPI.h>
#include <mcp3004.h>

//// Declarar variables globales spi_channel, pin_base...
#define spi_channel 1
#define MISO 13 
#define MOSI 12
#define SCLK 14
#define speed 50000
int pin_base = 100;
int frec;
int n_muestras = 100;
float tension[n_muestras];
int tiempo;
//// complementario: tiempo inicio bucle
int main() {
//// Configurar WiringPI
wiringPiSetup (void) ;
//// Configurar SPI
wiringPiSPISetup (spi_channel,speed) ;
//// Configurar mcp3004
mcp3004Setup(pin_base,spi_channel);
//// Lectura de frec de muestreo por consola

//// Calculo de periodo de muestreo
printf("Introduzca un entero para dar valor a la frecuencia (en kHz) de muestreo\n")
scanf("%d",&frec);
int t_muestreo = 1/(frec*1000);
//// Calcular inicio tiempo de lectura de pin
int i = 0;
int t_ini, t_fin;
int t_bucle_ini = milis();
int m_perdida = 0;
while(i<n_muestras){
    t_ini = micros()   ; 
    tension[i] = analogRead(pin_base)*3.3/1024;
    t_fin = micros();
    printf("%d \t %d \t %d \n",i,tension,t_fin-t_bucle_ini);
    t_fin = micros();
    t_muestreo = t_muestreo*10^6-t_fin+t_ini;
    if(t_muestreo<0){
        printf("!Muestra perdida!\n en la it");
        t_muestreo=0;
        m_perdida++;
    }
    delay(t_muestreo);
    i++;
}
int t_bucle_fin = milis();
printf("La frecuencia efectiva de muestreo son : %d kHz",n_muestras/(t_bucle_fin-t_bucle_ini))
//// Lectura del ADC
//// Calcular fin tiempo de lectura de pin
//// Ajuste de muestreo
//// Control de muestra perdida
//// Imprimir valor de iteración, tensión, tiempo
//// Efectuar retardo calculado
}
/// complementario: tiempo final bucle
/// complementario: cálculo de frecuencia efectiva de lectura