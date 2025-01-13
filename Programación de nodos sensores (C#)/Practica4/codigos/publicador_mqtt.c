#define _REENTRANT
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>

#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <mosquitto.h>
#include <string.h>

#include <wiringPiSPI.h>
#include <mcp3004.h>

#define spi_channel  0
#define pinbase 100
#define velocidad 5e4// 1 MHz

#define MAX_ROWS 70
#define MAX 10
#define LSM6DS33 0x6a


#define SWITCH1 31
#define LED1 24
#define LED2 28
// La macro TRAND genera un valor entero seudo-aleatorio 
// comprendido entre 1 y "limit" y lo almacena en "n". Para 
// favorecer la aleatoriedad usa como semilla el componente
// micro-segundos (usec) de la fecha-hora actual


void *get_temperature(void *);
void *get_distance(void *);

/* Callback called when the client receives a CONNACK message from the broker. */
void on_connect(struct mosquitto *mosq, void *obj, int reason_code)
{
	/* Print out the connection result. mosquitto_connack_string() produces an
	 * appropriate string for MQTT v3.x clients, the equivalent for MQTT v5.0
	 * clients is mosquitto_reason_string().
	 */
	printf("on_connect: %s\n", mosquitto_connack_string(reason_code));
	if(reason_code != 0){
		/* If the connection fails for any reason, we don't want to keep on
		 * retrying in this example, so disconnect. Without this, the client
		 * will attempt to reconnect. */
		mosquitto_disconnect(mosq);
	}

	/* You may wish to set a flag here to indicate to your application that the
	 * client is now connected. */
}


/* Callback called when the client knows to the best of its abilities that a
 * PUBLISH has been successfully sent. For QoS 0 this means the message has
 * been completely written to the operating system. For QoS 1 this means we
 * have received a PUBACK from the broker. For QoS 2 this means we have
 * received a PUBCOMP from the broker. */
void on_publish(struct mosquitto *mosq, void *obj, int mid)
{
	printf("Message with mid %d has been published.\n", mid);
}

void publish_sensor_temp(struct mosquitto *mosq, float temp)
{
	char payload[20];
	int rc;

	/* Get our pretend data */

	/* Print it to a string for easy human reading - payload format is highly
	 * application dependent. */
	snprintf(payload, sizeof(payload), "%lf", temp);

	/* Publish the message
	 * mosq - our client instance
	 * *mid = NULL - we don't want to know what the message id for this message is
	 * topic = "example/temperature" - the topic on which this message will be published
	 * payloadlen = strlen(payload) - the length of our payload in bytes
	 * payload - the actual payload
	 * qos = 2 - publish with QoS 2 for this example
	 * retain = false - do not use the retained message feature for this message
	 */
	rc = mosquitto_publish(mosq, NULL, "RaspberryPi/temperature", strlen(payload), payload, 2, false);
	if(rc != MOSQ_ERR_SUCCESS){
		fprintf(stderr, "Error publishing: %s\n", mosquitto_strerror(rc));
	}
}

float getDistance(float measure, float *x, float *voltage){
	float dist;

	int n = 0;
	
	while(n<MAX_ROWS){
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

void publish_sensor_distance(struct mosquitto *mosq, float dist)
{
	char payload[20];
	int rc;

	/* Get our pretend data */

	/* Print it to a string for easy human reading - payload format is highly
	 * application dependent. */
	snprintf(payload, sizeof(payload), "%lf", dist);

	/* Publish the message
	 * mosq - our client instance
	 * *mid = NULL - we don't want to know what the message id for this message is
	 * topic = "example/temperature" - the topic on which this message will be published
	 * payloadlen = strlen(payload) - the length of our payload in bytes
	 * payload - the actual payload
	 * qos = 2 - publish with QoS 2 for this example
	 * retain = false - do not use the retained message feature for this message
	 */
	rc = mosquitto_publish(mosq, NULL, "RaspberryPi/distance", strlen(payload), payload, 2, false);
	if(rc != MOSQ_ERR_SUCCESS){
		fprintf(stderr, "Error publishing: %s\n", mosquitto_strerror(rc));
	}
}

int main(int argc, char *argv[]) {
	struct mosquitto *mosq;
	
	mosquitto_lib_init();
	
    int rc;
    
    mosq = mosquitto_new(NULL, true, NULL);
	if(mosq == NULL){
		fprintf(stderr, "Error: Out of memory.\n");
		return 1;
	}

	/* Configure callbacks. This should be done before connecting ideally. */
	mosquitto_connect_callback_set(mosq, on_connect);
	mosquitto_publish_callback_set(mosq, on_publish);

	/* Connect to test.mosquitto.org on port 1883, with a keepalive of 60 seconds.
	 * This call makes the socket connection only, it does not complete the MQTT
	 * CONNECT/CONNACK flow, you should use mosquitto_loop_start() or
	 * mosquitto_loop_forever() for processing net traffic. */
	rc = mosquitto_connect(mosq, "test.mosquitto.org", 1883, 60);
	if(rc != MOSQ_ERR_SUCCESS){
		mosquitto_destroy(mosq);
		fprintf(stderr, "Error: %s\n", mosquitto_strerror(rc));
		return 1;
	}

	/* Run the network loop in a background thread, this call returns quickly. */
	rc = mosquitto_loop_start(mosq);
	if(rc != MOSQ_ERR_SUCCESS){
		mosquitto_destroy(mosq);
		fprintf(stderr, "Error: %s\n", mosquitto_strerror(rc));
		return 1;
	}
	
    wiringPiSetup();
	wiringPiSPISetup(spi_channel,velocidad); //Configuramos canal de comunicación y velocidad
    mcp3004Setup (pinbase, spi_channel);
    pthread_t thread1,thread2;

    if ((rc=pthread_create(&thread1,NULL,&get_temperature,(void *) mosq))){
	printf("Fallo de pthread:\n");
    }
    if ((rc=pthread_create(&thread2,NULL,&get_distance,(void *) mosq))){
	printf("Fallo de pthread:\n");
    }
    pthread_join(thread1,NULL);
    pthread_join(thread2,NULL);
    
    printf("From main proces IC: %d\n",(int)getpid());
    
    mosquitto_lib_cleanup();
    exit(0);
    return 0;
}

void *get_temperature(void *mosq)
{
    
    short int raw_data;
    unsigned char data_read[2];
    int fd  = wiringPiI2CSetup(LSM6DS33); 
    
    while(1){
    data_read[0] = wiringPiI2CReadReg8(fd,0x20);
    data_read[1] = wiringPiI2CReadReg8(fd,0x21);

    raw_data = (data_read[1]<<8)|(data_read[0]);

    float temp_data = (double)raw_data/16+25;
    
    //printf("temp=%f ºC\n",temp_data);
    publish_sensor_temp((struct mosquitto *) mosq,temp_data);
    
    delay(1e3);
}
return (void *) NULL;
}

void *get_distance(void *mosq){
	
FILE *file;

    float x[MAX_ROWS+1];
    float voltage[MAX_ROWS+1];
    int fila = 0;

    file = fopen("calibracion.csv","r");
    
    if (file == NULL) {
        printf("No se pudo abrir el archivo\n");
	exit(1);
    }

    // Leer el archivo CSV
    while (fila < MAX_ROWS) {
        fscanf(file,"%f;%f",&x[fila],&voltage[fila]);
        fila++;
    }

fclose(file);
	
//// Calculo de periodo de muestreo
float t_muestreo=1e3/(3); //Pasamos el tiempo de muestreo a microsegundos para compararlo con los de medida del ADC
float measure[2];
float alpha = 0.9;
float t1,t2;
float dist;

while(1) {
	t1 = millis();
	
	measure[0] = measure[1];
	measure[1] = analogRead(pinbase)*3.3/1023; //Paso a voltios 
	measure[1] = alpha*measure[1]+(1-alpha)*measure[0];
	
	dist = getDistance(measure[1],&x[0],&voltage[0]);
	
	//printf("distancia = %f cm \n",dist);
	publish_sensor_distance((struct mosquitto *) mosq,dist);
	t2 = millis();
	if(t_muestreo>(t2-t1))
	delay(t_muestreo-(t2-t1)); //EL tiempo entre medida y medida menos lo que tardamos en leer el pin, el retardo.

	}
return (void *) NULL;
}
