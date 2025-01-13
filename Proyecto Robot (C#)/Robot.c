#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <ctype.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <string.h>

#include <wiringPiSPI.h>
#include <mcp3004.h>
#include <math.h>

#define spi_channel  0
#define pbDstLft 101
#define pbDstRth 100
#define pbCntLft 103
#define pbCntRth 102

#define velocidad 5e4// 1 MHz

//#define MAX_ROWS 70
#define MAX 10
#define LSM6DS33 0x6a

#define Pin_D 23 // BCM 13, este pin es el qur ira en sentido antihorario para moverse hacia adelante
#define Pin_I 1 //BCM 18, este ira en sentido horario, pues es la salida para el motor derecho

float cuentas = 20;
float diametro = 71;
float eje = 198;
int dimension;


typedef struct {
    float distLeft;
    float distRight;
    int counterLeft;
    int counterRight;
    pthread_mutex_t mutexLeft;
    pthread_mutex_t mutexRight;
    pthread_mutex_t mutexEncoderLeft;
    pthread_mutex_t mutexEncoderRight;
    float *x;
    float *voltageLft;
	float *voltageRht;
	pthread_barrier_t barrera;
	int salir;
} ThreadArgs;

typedef struct{
	float x;
	float y;
	float theta;
} Position;

void motorDodgeRoutine(Position *actualPosition, void *args_ptr);
void modifyCoordinates(float x, float y, float theta, Position *position);
void motorMoveRelative(int security, float mm, void *args_ptr, Position *actualPosition);

// Función para inicializar la estructura compartida
ThreadArgs* initSharedData() {
    ThreadArgs *data = malloc(sizeof(ThreadArgs));
    if (data == NULL) {
        fprintf(stderr, "Error de asignación de memoria\n");
        exit(EXIT_FAILURE);
    }
    data->distLeft = 0.0;
    data->distRight = 0.0;
    data->counterLeft = 0;
    data->counterRight = 0;
	data->salir = 0;
    pthread_mutex_init(&data->mutexLeft, NULL);
    pthread_mutex_init(&data->mutexRight, NULL);
    pthread_mutex_init(&data->mutexEncoderLeft, NULL);
    pthread_mutex_init(&data->mutexEncoderRight, NULL);
    pthread_barrier_init(&data->barrera, NULL, 3);
    return data;
}

void destroySharedData(ThreadArgs *data) {
    pthread_mutex_destroy(&data->mutexLeft);
    pthread_mutex_destroy(&data->mutexRight);
    pthread_mutex_destroy(&data->mutexEncoderLeft);
    pthread_mutex_destroy(&data->mutexEncoderRight);
    pthread_barrier_destroy(&data->barrera);
    free(data->x);
	free(data->voltageLft);
	free(data->voltageRht);
	free(data);
}

float getDistance(float measure, float *x, float *voltage){
	float dist;
	int n = 0;
	while(n<dimension){
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

/*void chargeValue(float *x, float *voltage){
	FILE *file;
    file = fopen("calibracion.csv","r");
    
    if (file == NULL) {
        printf("No se pudo abrir el archivo\n");
	exit(1);
    }

    // Leer el archivo CSV
    while (fila < MAX_ROWS) {
        fscanf(file,"%f;%f",x,voltage);
        printf("%lf\t%lf\n",*x, *voltage);
        x++;
        voltage++;
    }
	fclose(file);
}*/

void calDistance(ThreadArgs *data){
	int precision;
	printf("Calibrando el vehículo. Por favor espere...\n");
	printf("Qué precisión en cm quiere que tenga la calibración: \n");
	scanf("%d",&precision);
	dimension = (int)(30/precision+1);
	printf("Dimension: %d\n",dimension);
	data->voltageLft = malloc(dimension* sizeof(float));
	if (data->voltageLft == NULL) {
		// Manejo de error si malloc falla
		perror("Error al asignar memoria para el array");
		exit(EXIT_FAILURE);
	}	
	data->voltageRht = malloc(dimension * sizeof(float));
	if (data->voltageRht == NULL) {
		// Manejo de error si malloc falla
		perror("Error al asignar memoria para el array");
		exit(EXIT_FAILURE);
	}	
	data->x = malloc(dimension * sizeof(float));
	if (data->x == NULL) {
		// Manejo de error si malloc falla
		perror("Error al asignar memoria para el array");
		exit(EXIT_FAILURE);
	}

	pthread_mutex_lock(&data->mutexEncoderLeft);
	data->counterLeft=0;
	pthread_mutex_unlock(&data->mutexEncoderLeft);

	pthread_mutex_lock(&data->mutexEncoderRight);
	data->counterRight=0;
	pthread_mutex_unlock(&data->mutexEncoderRight);
	int n, counterLeft, counterRight, counter,j;
	float t1;
	float measure[2] = {0.0,0.0};
	j=0;
	for(int i=precision;i<=30;i+=precision){
		n=0;
		
		counter = (int)cuentas*i*10/(M_PI*diametro);
		//printf("Counter: %d\n",counter);
		pwmWrite(Pin_D,320);
		pwmWrite(Pin_I,280);
		
		do
		{
			pthread_mutex_lock(&data->mutexEncoderLeft);
			counterLeft = data->counterLeft;
			pthread_mutex_unlock(&data->mutexEncoderLeft);

			pthread_mutex_lock(&data->mutexEncoderRight);
			counterRight = data->counterRight;
			pthread_mutex_unlock(&data->mutexEncoderRight);
		} while (counterLeft <= counter || counterRight <= counter);
		if(counterLeft<counterRight){
		pwmWrite(Pin_D,300);
		pwmWrite(Pin_I,280);
		}
		do
		{
			pthread_mutex_lock(&data->mutexEncoderLeft);
			counterLeft = data->counterLeft;
			pthread_mutex_unlock(&data->mutexEncoderLeft);

			pthread_mutex_lock(&data->mutexEncoderRight);
			counterRight = data->counterRight;
			pthread_mutex_unlock(&data->mutexEncoderRight);
		} while (counterLeft < counterRight);
		if(counterLeft>counterRight){
		pwmWrite(Pin_D,320);
		pwmWrite(Pin_I,300);
		}
		do
		{
			pthread_mutex_lock(&data->mutexEncoderLeft);
			counterLeft = data->counterLeft;
			pthread_mutex_unlock(&data->mutexEncoderLeft);

			pthread_mutex_lock(&data->mutexEncoderRight);
			counterRight = data->counterRight;
			pthread_mutex_unlock(&data->mutexEncoderRight);
		} while (counterLeft > counterRight);
		
		pwmWrite(Pin_I,300);
		pwmWrite(Pin_D,300);
		printf("Midiendo a %d cm...\n",i);

		t1 = millis();
		while(millis()-t1<5e3){
			measure[0] += analogRead(pbDstLft)*3.3/1023;
			measure[1] += analogRead(pbDstRth)*3.3/1023;
			delay(100);
			n++;
		}

		data->voltageRht[j] = measure[1]/n;
		data->voltageLft[j] = measure[0]/n;
		measure[0] = 0.0;	measure[1] = 0.0;
		printf("VL: %lf\tVR: %lf\tx: %d\n",data->voltageRht[j],data->voltageLft[j],i);
		data->x[j] = i;
		j++;
		if(j>dimension)printf("Tamaño de array insuficiente\n");
	}
	data->voltageRht[j] = 0.0;
	data->voltageLft[j] = 0.0;
	data->x[j] = 40;
	printf("Retire el objeto de calibración\n");
	int a;
	scanf("%d",&a);
	
	pthread_mutex_lock(&data->mutexEncoderLeft);
	data->counterLeft=0;
	pthread_mutex_unlock(&data->mutexEncoderLeft);

	pthread_mutex_lock(&data->mutexEncoderRight);
	data->counterRight=0;
	pthread_mutex_unlock(&data->mutexEncoderRight);	
}
	
void *distanceMeasureRigth(void *args_ptr){
	
	ThreadArgs *args = (ThreadArgs*)args_ptr;
	pthread_barrier_wait(&args->barrera);
    float *x = args->x;
    float *voltage = args->voltageRht;
  
	//SharedData *sharedData = (SharedData *)sharedData;
	
	float t_muestreo=1e3/(3); //Pasamos el tiempo de muestreo a microsegundos para compararlo con los de medida del ADC
	float measure[2];
	float alpha = 0.9;
	float t1,t2;
	float distRight;

	
	while(1) {
		t1 = millis();
		
		measure[0] = measure[1];
		measure[1] = analogRead(pbDstRth)*3.3/1023; //Paso a voltios 
		measure[1] = alpha*measure[1]+(1-alpha)*measure[0];
		
		distRight = getDistance(measure[1],x,voltage);
		pthread_mutex_lock(&args->mutexRight);
		args->distRight = distRight;
		pthread_mutex_unlock(&args->mutexRight);
        
		t2 = millis();
		if(t_muestreo>(t2-t1))
		delay(t_muestreo-(t2-t1)); //EL tiempo entre medida y medida menos lo que tardamos en leer el pin, el retardo.
		if(args->salir) pthread_exit(NULL);
		}
	
		
	return (void *) NULL;
}

void *distanceMeasureLeft(void *args_ptr){
	
	ThreadArgs *args = (ThreadArgs*)args_ptr;
	pthread_barrier_wait(&args->barrera);
    float *x = args->x;
    float *voltage = args->voltageLft;
    
    //SharedData *sharedData = (SharedData *)sharedData;
   
	float t_muestreo=1e3/(3); //Pasamos el tiempo de muestreo a microsegundos para compararlo con los de medida del ADC
	float measure[2];
	float alpha = 0.9;
	float t1,t2;
	float distLeft;

	while(1) {
		t1 = millis();
		
		measure[0] = measure[1];
		measure[1] = analogRead(pbDstLft)*3.3/1023; //Paso a voltios 
		measure[1] = alpha*measure[1]+(1-alpha)*measure[0];
		
		distLeft = getDistance(measure[1],x,voltage);
		pthread_mutex_lock(&args->mutexLeft);
		args->distLeft = distLeft;
		pthread_mutex_unlock(&args->mutexLeft);
        
	
		t2 = millis();
		if(t_muestreo>(t2-t1))
		delay(t_muestreo-(t2-t1)); //EL tiempo entre medida y medida menos lo que tardamos en leer el pin, el retardo.
		if(args->salir) pthread_exit(NULL);
		}
		
	return (void *) NULL;
}

void motorMoveRelative(int security, float mm, void *args_ptr, Position *actualPosition){
	
	printf("Movieno el vehículo en linea recta %f mm. El vehiculo se detendrá si se detecta un obstáculo a menos de %d cm.\n",mm, security);
	ThreadArgs *data = (ThreadArgs*)args_ptr;
	
	float distLeft, distRight;
	int counterLeft, counterRight;
	int counter = (int)cuentas*mm/(M_PI*diametro);
	
	pthread_mutex_lock(&data->mutexEncoderLeft);
	data->counterLeft=0;
	pthread_mutex_unlock(&data->mutexEncoderLeft);

	pthread_mutex_lock(&data->mutexEncoderRight);
	data->counterRight=0;
	pthread_mutex_unlock(&data->mutexEncoderRight);
	
	do
	{
		pthread_mutex_lock(&data->mutexLeft);
		distLeft = data->distLeft;
		pthread_mutex_unlock(&data->mutexLeft);

		pthread_mutex_lock(&data->mutexRight);
		distRight = data->distRight;
		pthread_mutex_unlock(&data->mutexRight);
		
		pthread_mutex_lock(&data->mutexEncoderLeft);
		counterLeft = data->counterLeft;
		pthread_mutex_unlock(&data->mutexEncoderLeft);

		pthread_mutex_lock(&data->mutexEncoderRight);
		counterRight = data->counterRight;
		pthread_mutex_unlock(&data->mutexEncoderRight);
		
		if(counterLeft-counterRight<-3){
		pwmWrite(Pin_D,280);
		pwmWrite(Pin_I,330);
		}
		if(counterLeft-counterRight>3){
		pwmWrite(Pin_D,270);
		pwmWrite(Pin_I,320);
		}
		if(counterLeft==counterRight){
		pwmWrite(Pin_I,330);
		pwmWrite(Pin_D,270);
		}
	} while (counter >= counterLeft && counter >= counterRight && distLeft >= security && distRight >= security);
	
	if(distLeft > security && distRight >security){
	if(counterLeft<counterRight){
		pwmWrite(Pin_D,300);
		pwmWrite(Pin_I,320);
		
	do{
		pthread_mutex_lock(&data->mutexEncoderLeft);
		counterLeft = data->counterLeft;
		pthread_mutex_unlock(&data->mutexEncoderLeft);

		pthread_mutex_lock(&data->mutexEncoderRight);
		counterRight = data->counterRight;
		pthread_mutex_unlock(&data->mutexEncoderRight);
		//printf("Distancia izquierda: %f, Dist derecha: %f.\n",distLeft,distRight);
		//delay(100);
	}while(counterLeft<counterRight);}
	if(counterLeft>counterRight){
		pwmWrite(Pin_D,280);
		pwmWrite(Pin_I,300);
		
	do{
		pthread_mutex_lock(&data->mutexEncoderLeft);
		counterLeft = data->counterLeft;
		pthread_mutex_unlock(&data->mutexEncoderLeft);

		pthread_mutex_lock(&data->mutexEncoderRight);
		counterRight = data->counterRight;
		pthread_mutex_unlock(&data->mutexEncoderRight);
		//printf("Distancia izquierda: %f, Dist derecha: %f.\n",distLeft,distRight);
		//delay(100);
	}while(counterLeft>counterRight);}
}

	pwmWrite(Pin_I,300);
	pwmWrite(Pin_D,300);

	modifyCoordinates(actualPosition->x+((float)(counterLeft)+(float)(counterRight))*M_PI*diametro/40*sin(actualPosition->theta*M_PI/180),actualPosition->y+((float)(counterLeft)+(float)(counterRight))*M_PI*diametro/40*cos(actualPosition->theta*M_PI/180),actualPosition->theta, actualPosition);
	
	
	pthread_mutex_lock(&data->mutexEncoderLeft);
	data->counterLeft=0;
	pthread_mutex_unlock(&data->mutexEncoderLeft);

	pthread_mutex_lock(&data->mutexEncoderRight);
	data->counterRight=0;
	pthread_mutex_unlock(&data->mutexEncoderRight);
	
	delay(2000);
}

void motorStop(){
	printf("Destino Alcanzado\n");
	pwmWrite(Pin_I,0);
	pwmWrite(Pin_D,0);
	//delay(6000);
}

void motorRotateAngle(float angle, void *args_ptr, Position *actualPosition){
	
	printf("Girando el vehículo. El vehiculo se detendrá cuando haya completado un giro de %fº.\n",angle);
	ThreadArgs *data = (ThreadArgs*)args_ptr;
	if(angle!=0){
	pthread_mutex_lock(&data->mutexEncoderLeft);
	data->counterLeft=0;
	pthread_mutex_unlock(&data->mutexEncoderLeft);

	pthread_mutex_lock(&data->mutexEncoderRight);
	data->counterRight=0;
	pthread_mutex_unlock(&data->mutexEncoderRight);
	
	int counter = (int)(abs(angle)*eje/360)*cuentas/diametro;
	int counterLeft, counterRight;
	if(angle<0){
		pwmWrite(Pin_I,270);
		pwmWrite(Pin_D,270);
	}else{
		pwmWrite(Pin_I,330);
		pwmWrite(Pin_D,330);
	}
	
	do
	{
		pthread_mutex_lock(&data->mutexEncoderLeft);
		counterLeft = data->counterLeft;
		pthread_mutex_unlock(&data->mutexEncoderLeft);

		pthread_mutex_lock(&data->mutexEncoderRight);
		counterRight = data->counterRight;
		pthread_mutex_unlock(&data->mutexEncoderRight);
	} while (counter >= counterLeft && counter>= counterRight);
	
	pwmWrite(Pin_I,300);
	pwmWrite(Pin_D,300);
	
	modifyCoordinates(actualPosition->x, actualPosition->y, actualPosition->theta+angle/abs(angle)*((float)(counterLeft)/2+(float)(counterRight)/2)*360*diametro/cuentas/eje, actualPosition);
	
	pthread_mutex_lock(&data->mutexEncoderLeft);
	data->counterLeft=0;
	pthread_mutex_unlock(&data->mutexEncoderLeft);

	pthread_mutex_lock(&data->mutexEncoderRight);
	data->counterRight=0;
	pthread_mutex_unlock(&data->mutexEncoderRight);
}
}

void *encoderLeft(void *args_ptr){
	
	//SharedData *sharedData = (SharedData *)sharedData;
	ThreadArgs *sharedData = (ThreadArgs*)args_ptr;
	
	float t_muestreo=30; //Pasamos el tiempo de muestreo a microsegundos para compararlo con los de medida del ADC
	int measure[2]={0,0};
	float t1,t2;
	//int counterLeft;
	
	while(1) {
		t1 = millis();
		measure[0]=measure[1];
		if(analogRead(pbCntLft)<=40) measure[1] = 0;
		if(analogRead(pbCntLft)>=50) measure[1] = 1;
		if(abs(measure[1]-measure[0])==1){
	
			pthread_mutex_lock(&sharedData->mutexEncoderLeft);
			sharedData->counterLeft++;
			//counterLeft=sharedData->counterLeft;
			pthread_mutex_unlock(&sharedData->mutexEncoderLeft);
			//printf("CounterLeft: %d\n",counterLeft);
		}
		//printf("MeasureLeft:\t%lf\n",measure[1]);
		t2 = millis();
		if(t_muestreo>(t2-t1))
		delay(t_muestreo-(t2-t1)); //EL tiempo entre medida y medida menos lo que tardamos en leer el pin, el retardo.
		if(sharedData->salir) pthread_exit(NULL);
		}
		
	return (void *) NULL;
}

void *encoderRigth(void *args_ptr){  
	
	//SharedData *sharedData = (SharedData *)sharedData;
	ThreadArgs *sharedData = (ThreadArgs*)args_ptr;
	
	float t_muestreo=30; //Pasamos el tiempo de muestreo a microsegundos para compararlo con los de medida del ADC
	int measure[2]={0,0};
	float t1,t2;
	//int counterRight;
	while(1) {
		t1 = millis();
		measure[0]=measure[1];
		if(analogRead(pbCntRth)<=250) measure[1] = 0;
		if(analogRead(pbCntRth)>=400) measure[1] = 1;
		if(abs(measure[1]-measure[0])==1){
			
			pthread_mutex_lock(&sharedData->mutexEncoderRight);
			sharedData->counterRight++;
			//counterRight=sharedData->counterRight;
			pthread_mutex_unlock(&sharedData->mutexEncoderRight);
			//printf("CounterRigth: %d\n",counterRight);
		}
		//printf("MeasureRigth:\t%lf\n",measure[1]);
		t2 = millis();
		if(t_muestreo>(t2-t1))
		delay(t_muestreo-(t2-t1)); //EL tiempo entre medida y medida menos lo que tardamos en leer el pin, el retardo.
		if(sharedData->salir) pthread_exit(NULL);
		}
		
	return (void *) NULL;
}

Position *definePosition(){
	Position *r = malloc(sizeof(Position));
	if(r==NULL){
		exit(EXIT_FAILURE);
	}
	r->x = 0.0;
	r->y = 0.0;
	r->theta = 0.0;
	return r;
}
	
void modifyCoordinates(float x, float y, float theta, Position *position){
	position->x = x;
	position->y = y;
	position->theta = theta;
	printf("Posición: x=%f, y=%f, theta=%f\n",position->x, position->y,position->theta);
}

typedef struct 
{
  int x;
  int y;
   
}Punto; //El punto 

typedef struct 
{
  int filas;
  int columnas;
  int *mapa;
   
}Mapper; //El punto 

Punto* create_point(int columna, int fila)
{
    Punto* point=(Punto*)malloc(sizeof(Punto)); //Reserva de meoria dinamica
    point->x=columna;
    point->y=fila;
     return point;

}

int optimizacion_vertices(int x_vertice_1, int y_vertice_1,Punto *final,Punto* punto_medio,int k, int* contador_vertices){ 
//Se calcula cual de los dos vertices encontrados minimiza la distancia
float distance_1=sqrt((x_vertice_1-final->x)*(x_vertice_1-final->x)+(y_vertice_1-final->y)*(y_vertice_1-final->y));
float distance_2=sqrt((punto_medio->x-final->x)*(punto_medio->x-final->x)+(punto_medio->y-final->y)*(punto_medio->y-final->y));

if (distance_1<distance_2)
{
punto_medio->x=x_vertice_1;
punto_medio->y=y_vertice_1;
printf("Se ha actualizado el punto medio (%d, %d).\n",punto_medio->x,12-punto_medio->y);
k+=2;
(*contador_vertices)++;
//delay(2000);
}
//No devolvemos nada, simplemente actualizamos el punto al que se va a ir en la estructura punto_medio
return k;
}

float* save_point(float* camino,int k, Punto* punto_medio){
    //Esta funcion tiene como objetivo aumentar el tamaño del arreglo y guardar el ultimo vertice
    float *temp = realloc(camino, k* sizeof(float));
    if (temp == NULL) {
        printf("Error al redimensionar el arreglo.");
        free(camino);
        return NULL;
    }
    camino = temp;
    //Multiplicamos por 100 para pasar de casillas a mm 
    //Una casilla es 10 cm
    camino[k-2]=punto_medio->x*100;
    camino[k-1]=(12-punto_medio->y)*100;

    return camino;
}

void modifyMapper(Mapper* mapper, Punto* centro, Punto* dimension, float theta) {
    centro->x += 2*sin(theta);
    centro->y += 2*cos(theta);

    for (int i = centro->x; i <= centro->x+dimension->x; i++) {
        
        for (int j = mapper->filas -1 - centro->y; j >= mapper->filas - 1 - centro->y - dimension->y; j--) {
            mapper->mapa[j*mapper->columnas+i] = 1;
            //printf("coordenadas (x y) = (%d %d)\n",i,j);
        }
         if (centro->x+dimension->x == i || centro->x == i) {
                mapper->mapa[(mapper->filas - 1 - centro->y - dimension->y)*mapper->columnas+i] = 2;
                mapper->mapa[(mapper->filas - 1 - centro->y)*mapper->columnas+i] = 2;
            }
    }

    // Imprimimos el mapa para verificar los cambios
    printf("\nMapa modificado:\n");
    for (int i = 0; i < mapper->filas; i++) {
        for (int j = 0; j < mapper->columnas; j++) {
            printf("%d ", mapper->mapa[mapper->columnas*i+j]);
        }
        printf("\n");
    }
}

void esquiva_vertical(Punto* punto_medio,Punto* final,Mapper* mapa,int delta_y){ //caso en el cual necesitamos realizar una esquiva vertical
    
   //int origen_y=punto_medio->y;
    
    if(delta_y<0){
        //Escaneo para obtener el punto de contaco
        for(int i=punto_medio->y; mapa->mapa[(i+1)*mapa->columnas+punto_medio->x]==0;i--){
            punto_medio->y=i; //Vamos actualizando verticalmente hasta encontrar el borde del objeto           

            }
        for(int j=punto_medio->x; j<mapa->columnas;j++){
            if(mapa->mapa[punto_medio->y*mapa->columnas+j]==2){
                //Se encontro un vertice

                punto_medio->x=j;
                break;}
           }
        }
    
    else if (delta_y>0){
      //Escaneo para obtener el punto de contaco
        for(int i=punto_medio->y; mapa->mapa[(i-1)*mapa->columnas+punto_medio->x]==0;i++){
            punto_medio->y=i; //Vamos actualizando verticalmente hasta encontrar el borde del objeto           
            }
        for(int j=punto_medio->x; j<mapa->columnas;j++){
            if(mapa->mapa[punto_medio->y*mapa->columnas+j]==2){
                //Se encontro un vertice
                punto_medio->x=j;
                break;}
    }
    
}
}

void esquiva_horizontal(Punto* punto_medio,Punto* final,Mapper* mapa,int delta_x){ //caso en el cual necesitamos realizar una esquiva vertical
    
   //int origen_y=punto_medio->y;
    
    if(delta_x<0){
        //Escaneo para obtener el punto de contaco
        for(int i=punto_medio->x; mapa->mapa[punto_medio->y*mapa->columnas+i+1]==0;i--){
            punto_medio->x=i; //Vamos actualizando verticalmente hasta encontrar el borde del objeto           
            }
        for(int j=punto_medio->y; j<mapa->filas;j++){
            if(mapa->mapa[j*mapa->columnas+punto_medio->x]==2){
                //Se encontro un vertice
                punto_medio->y=j;
                break;}
           }
        }
    
    else if (delta_x>0){
      //Escaneo para obtener el punto de contaco
        for(int i=punto_medio->x; mapa->mapa[punto_medio->y*mapa->columnas+i-1]==0;i++){
            punto_medio->x=i; //Vamos actualizando verticalmente hasta encontrar el borde del objeto           
            }
        for(int j=punto_medio->y; j<mapa->filas;j++){
            if(mapa->mapa[j*mapa->columnas+punto_medio->x]==2){
                //Se encontro un vertice
                punto_medio->y=j;
                break;}
           }
   }
}

/*void motorDodgeRoutine(Position *actualPosition, void *args_ptr, Mapper *mapa){
	motorRotateAngle(-90, args_ptr, actualPosition);
	motorMoveRelative(0, 400, args_ptr, actualPosition);
	motorRotateAngle(90, args_ptr, actualPosition);
	motorMoveRelative(0, 500, args_ptr, actualPosition);
	Punto dim = {4, 4}
	modifyMapper(mapa,actualPosition,&dim);
}*/

void generateInstructions(Position *actualPosition, Position *nextPosition, void *args_ptr, int security, int *k){
	printf("Angle1: %f\nDist: %f\n",atan2(nextPosition->x-actualPosition->x, nextPosition->y-actualPosition->y)*180/M_PI-actualPosition->theta,sqrt((nextPosition->x-actualPosition->x)*(nextPosition->x-actualPosition->x)+(nextPosition->y-actualPosition->y)*(nextPosition->y-actualPosition->y)));
	motorRotateAngle(atan2(nextPosition->x-actualPosition->x, nextPosition->y-actualPosition->y)*180/M_PI-actualPosition->theta,args_ptr, actualPosition);
	delay(1000);
	motorMoveRelative(security, sqrt((nextPosition->x-actualPosition->x)*(nextPosition->x-actualPosition->x)+(nextPosition->y-actualPosition->y)*(nextPosition->y-actualPosition->y)), args_ptr, actualPosition);
	
	ThreadArgs *data = (ThreadArgs*) args_ptr;
	float distLeft, distRight;
	
	pthread_mutex_lock(&data->mutexLeft);
	distLeft = data->distLeft;
	pthread_mutex_unlock(&data->mutexLeft);

	pthread_mutex_lock(&data->mutexRight);
	distRight = data->distRight;
	pthread_mutex_unlock(&data->mutexRight);

	if(distLeft <= security || distRight <= security){
	printf("Obstaculo cercano detectado(Rht: %f, Lft: %f)\n",distRight,distLeft);
	*k=0;
	pwmWrite(Pin_I,300);
	pwmWrite(Pin_D,300);
	delay(1000);

	}

	else if(sqrt((actualPosition->x-nextPosition->x)*(actualPosition->x-nextPosition->x) + (actualPosition->y-nextPosition->y)*(actualPosition->y-nextPosition->y))>100){
		printf("Generando de nuevo instrucciones\n");
		generateInstructions(actualPosition, nextPosition, args_ptr, security, k);}
	}

float* calculo_camino(Punto *origen,Punto* final,Punto* punto_medio,Mapper* mapa,int* N, void * args_ptr, Position *actualPosition) { //Este algoritmo ira buscando los vertices mas cercanos
    printf("Cálculo de camino\n");
    int contador_vertices;
    int delta_x, delta_y;
    int k=0;
    float* camino=NULL;//incializamos el puntero  NULL
    //Hacemos una primera reserva de memoria
    camino=(float*)(malloc(2*sizeof(float)));
    //Se buscan los dos proximos vertices y se evalua cual es mas optimo

    //Hay que implementar algoritmo que calcule el proximo punto y lo actualice 
    while (punto_medio->x != final->x || punto_medio->y != final->y) { //Mientras no estemos en el punto con un error de 10 cm
    //Volvemos a 
    delta_x=final->x-punto_medio->x;
    delta_y=final->y-punto_medio->y;
    contador_vertices=0;


        if((delta_x<=0)&&(delta_y<=0)) {
        //En este caso estamos por encima y a la derecha del punto que se desea alcanzar
        if(delta_x==0 && delta_y!=0){
            //Rutina de esquiva vertical
            esquiva_vertical(punto_medio,final,mapa,delta_y);
            contador_vertices++;
            k=+2;
            camino=save_point(camino,k,punto_medio);
            printf("Se ha actualizado el punto medio (%d, %d).\n",punto_medio->x,12-punto_medio->y);
            delay(2000);
            }
            
         else if(delta_x!=0 && delta_y==0){//Rutina esquiva horizontal
             esquiva_horizontal(punto_medio,final,mapa,delta_x);
             contador_vertices++;
             k=+2;
             camino=save_point(camino,k,punto_medio);
             printf("Se ha actualizado el punto medio (%d, %d).\n",punto_medio->x,12-punto_medio->y);
             delay(2000);
         } 
            
        else{
            
        for(int i=punto_medio->y;i>=0;i--) //barrido en filas
        {
            for(int j=punto_medio->x;j>=final->x;j--){//barrido en columnas

                if ((mapa->mapa[i*mapa->columnas+j]==0)&&(mapa->mapa[i*mapa->columnas+j+1]==1))
                {//salida del bucle si se detecta un objeto siendo atravesado horizontalmente 
                    break;
                }
                else if((mapa->mapa[i*mapa->columnas+j]==2)&&(j!=punto_medio->x || i!=punto_medio->y)){//estamos en el primer vertice
                k=optimizacion_vertices(j,i,final,punto_medio,k,&contador_vertices);
                camino=save_point(camino,k,punto_medio);
                }
            }
        }
    }
}

        else if((delta_x<=0)&&(delta_y>=0)) {
            
        if(delta_x==0 && delta_y!=0){
            //Rutina de esquiva vertical
            esquiva_vertical(punto_medio,final,mapa,delta_y);
             contador_vertices++;
             k=+2;
             camino=save_point(camino,k,punto_medio);
             printf("Se ha actualizado el punto medio (%d, %d).\n",punto_medio->x,12-punto_medio->y);
             delay(2000);
            }
            
         else if(delta_x!=0 && delta_y==0){//Rutina esquiva horizontal
             esquiva_horizontal(punto_medio,final,mapa,delta_x);
              contador_vertices++;
              k=+2;
              camino=save_point(camino,k,punto_medio);
              printf("Se ha actualizado el punto medio (%d, %d).\n",punto_medio->x,12-punto_medio->y); 
              delay(2000);           
         }    
        //En este caso estamos por debajo y a la derecha del punto que se desea alcanzar
        else 
        {for(int i=punto_medio->y;i<=mapa->filas-1;i++) //barrido en filas
        {
            for(int j=punto_medio->x;j>=final->x;j--){//barrido en columnas

                if ((mapa->mapa[i*mapa->columnas+j]==1)&&(mapa->mapa[i*mapa->columnas+j+1]==0))
                {//salida del bucle si se detecta un objeto siendo atravesado horizontalmente 
                    break;
                }
                else if((mapa->mapa[i*mapa->columnas+j]==2)&&(j!=punto_medio->x || i!=punto_medio->y)){//estamos en el primer vertice
                
                k=optimizacion_vertices(j,i,final,punto_medio,k,&contador_vertices);
                camino=save_point(camino,k,punto_medio);
                }
            }
        }
        }
    }

        else if ((delta_x>=0)&&(delta_y>=0)) //Estamos aqui
        {
        if(delta_x==0 && delta_y!=0){
            //Rutina de esquiva vertical
             esquiva_vertical(punto_medio,final,mapa,delta_y);
             contador_vertices++;
             k=+2;
             camino=save_point(camino,k,punto_medio);
             printf("Se ha actualizado el punto medio (%d, %d).\n",punto_medio->x,12-punto_medio->y);  
             delay(2000);          
            }
            
        else if(delta_x!=0 && delta_y==0){//Rutina esquiva horizontal
             esquiva_horizontal(punto_medio,final,mapa,delta_x);
             contador_vertices++;
             k=+2;
             camino=save_point(camino,k,punto_medio);
             printf("Se ha actualizado el punto medio (%d, %d).\n",punto_medio->x,12-punto_medio->y);
             //delay(2000);
         }    
        //En este caso estamos por debajo y a la izquierda del punto que se desea alcanzar
        else {
            
        for(int i=punto_medio->y;i<=mapa->filas-1;i++) //barrido en filas
        {
            for(int j=punto_medio->x;j<=final->x;j++){//barrido en columnas
                     
                if ((mapa->mapa[i*mapa->columnas+j]==0)&&(mapa->mapa[i*mapa->columnas+j+1]==1))
                {//salida del bucle si se detecta un objeto siendo atravesado horizontalmente 
                    
                    break;
                }
                else if((mapa->mapa[i*mapa->columnas+j]==2)&&(j!=punto_medio->x || i!=punto_medio->y)){//estamos en el primer vertice
                printf("Hemos entrado\n");
                k=optimizacion_vertices(j,i,final,punto_medio,k,&contador_vertices);
                camino=save_point(camino,k,punto_medio);
                
                }
            }
        }
        }
    }

        else if ((delta_x>=0)&&(delta_y<=0))
        {
        if(delta_x==0 && delta_y!=0){
            //Rutina de esquiva vertical
            esquiva_vertical(punto_medio,final,mapa,delta_y);
            contador_vertices++;
             k=+2;
             camino=save_point(camino,k,punto_medio);
             printf("Se ha actualizado el punto medio (%d, %d).\n",punto_medio->x,12-punto_medio->y);
             delay(2000);
            }
            
         else if(delta_x!=0 && delta_y==0){//Rutina esquiva horizontal
             esquiva_horizontal(punto_medio,final,mapa,delta_x);
             contador_vertices++;
             k=+2;
             camino=save_point(camino,k,punto_medio);
             printf("Se ha actualizado el punto medio (%d, %d).\n",punto_medio->x,12-punto_medio->y);
             delay(2000);
         }    
        //En este caso estamos por arriba y a la izquierda del punto que se desea alcanzar
        else {
        for(int i=punto_medio->y;i>=0;i--) //barrido en filas
        {
            for(int j=punto_medio->x;j<=final->x;j++){//barrido en columnas

                if ((mapa->mapa[i*mapa->columnas+j]==0)&&(mapa->mapa[i*mapa->columnas+j+1]==1))
                {//salida del bucle si se detecta un objeto siendo atravesado horizontalmente 
                    break;
                }
                else if((mapa->mapa[i*mapa->columnas+j]==2)&&(j!=punto_medio->x || i!=punto_medio->y)){//estamos en el primer vertice
                
                k=optimizacion_vertices(j,i,final,punto_medio,k,&contador_vertices);
                camino=save_point(camino,k,punto_medio);
                }
            }
        }
        }
    }
           



        if (contador_vertices==0)
        {   k+=2;
            printf("No se han detectado vertices.\n");
            printf("Se ira directamente al punto final: (%d,%d), partiendo de (%d,%d).\n",final->x,12-final->y,punto_medio->x,12-punto_medio->y);
            camino=save_point(camino,k,final);
            punto_medio->x=final->x;//
            punto_medio->y=final->y;//Se actualiza condicion salida bucle
            delay(3000);
            
            

        }
        
    }

    int i=0;
    
    Position *nextPosition = definePosition();
    //modifyCoordinates(origen->x*25,(12-origen->y)*25,0, actualPosition);
    do{
	modifyCoordinates(camino[i],camino[i+1],0, nextPosition);
	i+=2;
	generateInstructions(actualPosition, nextPosition, args_ptr, 20, &k);
    } while(i<k && k!=0);

    
    if(k==0){   
	origen->x = actualPosition->x/100;
	origen->y = actualPosition->y/100;
	Punto *dim = create_point(4,4);
	modifyMapper(mapa, origen, dim, actualPosition->theta);
	free(dim);
	motorRotateAngle(nextPosition->theta-actualPosition->theta, args_ptr, actualPosition);
	origen->x = actualPosition->x/100;
	origen->y = 12 - actualPosition->y/100;
	//free(actualPosition);
	free(nextPosition);
	*N=0;
	calculo_camino(origen, final, origen, mapa, N, args_ptr, actualPosition);}
else{
    printf("En principio ya se ha alcanzado el punto objetivo.\n");
    *N=k;
    //free(actualPosition);
    free(nextPosition);}
    return camino;

}

void chargeMap(Mapper *mapa){
    FILE *archivo;
    char buffer[100];
    mapa->filas = 0;
    mapa->columnas = 0;

    archivo = fopen("map.txt", "r");

    if (archivo == NULL) {
        printf("No se pudo abrir el archivo.\n");
        exit(EXIT_FAILURE);
    }

    while (fgets(buffer, sizeof(buffer), archivo)) {
        char *ptr = buffer;
        while (*ptr != '\0') {
            while (isspace(*ptr)) {
                ptr++;
            }
            if (isdigit(*ptr)) {
                mapa->columnas++;
                while (*ptr != '\0' && !isspace(*ptr)){ptr++;}
            } else {
                ptr++;
            }
        }
        mapa->filas++;
    }
    mapa->columnas=mapa->columnas/mapa->filas;
    printf("columnas: %d, filas: %d\n",mapa->columnas, mapa->filas);
    fseek(archivo, 0, SEEK_SET);

    mapa->mapa = (int *)malloc(mapa->filas*mapa->columnas * sizeof(int));

    for (int i = 0; i < mapa->filas*mapa->columnas; i++) {
        if (fscanf(archivo, "%d", &mapa->mapa[i]) != 1) {
            printf("Error al leer los datos del archivo.\n");
            fclose(archivo);
            free(mapa->mapa);
            exit(EXIT_FAILURE);
        }
    }

    fclose(archivo);

    
    printf("Datos almacenados en el mapa cargado:\n");
    for (int i = 0; i < mapa->filas*mapa->columnas; i++) {
            if(i%mapa->columnas==0 && i>0) printf("\n");
            printf("%d ", mapa->mapa[i]);
    }
    printf("\n");
}

Mapper *defineMapper(){
    Mapper *r = malloc(sizeof(Mapper));
    if (r == NULL) {
        printf("Error: No se pudo asignar memoria para Point3D.\n");
        exit(EXIT_FAILURE);
    }
    chargeMap(r);
    return r;
}

void *movement(void *args_ptr){
	calDistance(args_ptr);

	pwmWrite(Pin_I,300);
	pwmWrite(Pin_D,300);
	
	ThreadArgs *data = (ThreadArgs*)args_ptr;
	pthread_barrier_wait(&data->barrera);
	/**Experimento 0 **//**
	Position *actualPosition = definePosition();
	modifyCoordinates(0,0,0, actualPosition);
	motorRotateAngle(180, args_ptr,actualPosition);
	motorMoveRelative(0,1290, args_ptr,actualPosition);
	free(actualPosition);**/
	
	/**Experimento 1 **//**
    char commands[] = "D800, R-45, D400";
    char *token;
    char *delimiter = ", ";
    Position *actualPosition = definePosition();
    modifyCoordinates(0,0,0, actualPosition);
    
    // Reiniciar strtok para volver a procesar la cadena
    token = strtok(commands, delimiter);
	char letra;
	int valor;
    // Procesar la cadena y almacenar los datos en el array Datos
    while (token != NULL) {
        sscanf(token, "%c%d", &letra, &valor);
        switch(letra) {
        case 'D':
            motorMoveRelative(0,valor,args_ptr, actualPosition);
            break;
        case 'R':
            motorRotateAngle(valor, args_ptr, actualPosition);
            break;
        default:
	    motorStop();
            printf("Error al leer las órdenes de movimiento. Formato no válido.\n");
            break;
    }
        token = strtok(NULL, delimiter);
    }
        free(actualPosition);**/
    
    /**Experimento 2 **//**
    Position *actualPosition = definePosition();
    Position *nextPosition = definePosition();
    modifyCoordinates(0,0,0, actualPosition);
    modifyCoordinates(800,800,-90, nextPosition);
    int ptr_value = -1;
    generateInstructions(actualPosition, nextPosition, args_ptr, 0, &ptr_value);
    
    free(actualPosition);
    free(nextPosition);**/
    
    /**Experimento 3 **/
    Mapper* mapa = defineMapper();
int N=0;
    int startX, startY, goalX, goalY;
    printf("Ingrese las coordenadas de inicio (x y): ");
    scanf("%d %d", &startX, &startY);
    startY=mapa->filas-1-startY;
    if((mapa->mapa[startY*mapa->columnas+startX])!=0){
	printf("Ese punto no es válido pues se encuentra en un objeto.\n");
	printf("Ingrese de nuevo las coordenadas de inicio (x y): ");
    scanf("%d %d", &startX, &startY);
    startY=mapa->filas-1-startY;}
    printf("Ingrese las coordenadas del objetivo (x y): ");
    scanf("%d %d", &goalX, &goalY);
    goalY=12-goalY;
    if((mapa->mapa[goalY*mapa->columnas+goalX])!=0){
	printf("Ese punto no es válido pues se encuentra en un objeto.\n");
	printf("Ingrese de nuevo las coordenadas del objetivo (x y): ");
    scanf("%d %d", &goalX, &goalY);
    goalY=mapa->filas-1-goalY;}
    
    Punto* origen=create_point(startX,startY);
    Punto* punto_medio=create_point(startX,startY);
    Punto* final=create_point(goalX,goalY);
    Position* actualPosition = definePosition();
    modifyCoordinates(origen->x*100,(12-origen->y)*100,0, actualPosition);
    
    float* camino=calculo_camino(origen,final,punto_medio,mapa,&N, args_ptr,actualPosition);
    printf("Puntos del camino: {");
    for (int i = 0; i <N; i+=2)
    {
        printf(" (%f, %f)",camino[i],camino[i+1]);
    }
    printf("}\n");

    free(actualPosition);
    free(mapa->mapa);
    free(mapa);
    free(origen);
    free(punto_medio);
    free(final);
    
    /**Experimento 4 **//**
    Position *actualPosition = definePosition();
    Position *nextPosition = definePosition();
    modifyCoordinates(0,0,0, actualPosition);
    modifyCoordinates(0,0,0, nextPosition);
    //motorDodgeRoutine(actualPosition, args_ptr);
    generateInstructions(actualPosition, nextPosition, args_ptr, 20, NULL);
    
    free(actualPosition);
    free(nextPosition);**/
    
	motorStop();	
	data->salir = 1;
	pthread_exit(NULL);
	return (void *) NULL;
}

int main(int argc, char *argv[]) {
	
	ThreadArgs *sharedData = initSharedData();
	//SharedData *sharedData = initSharedData();
	int rc;
    
    wiringPiSetup();
    
    pinMode(Pin_D,PWM_OUTPUT);
	pinMode(Pin_I,PWM_OUTPUT);
	pwmSetMode(PWM_MODE_MS);
	pwmSetRange(4000);
	pwmSetClock(96);
	

    wiringPiSPISetup(spi_channel,velocidad); //Configuramos canal de comunicación y velocidad
    mcp3004Setup (100, spi_channel);
    //mcp3004Setup (pbDstRth, spi_channel);

    pthread_t thread1,thread2,thread3,thread4,thread5;	
    
    if ((rc=pthread_create(&thread1,NULL,&distanceMeasureRigth,sharedData))){
	printf("Fallo de pthread:\n");
    }
    if ((rc=pthread_create(&thread2,NULL,&distanceMeasureLeft,sharedData))){
	printf("Fallo de pthread:\n");
    }
    if ((rc=pthread_create(&thread3,NULL,&encoderLeft,sharedData))){
	printf("Fallo de pthread:\n");
    }
    if ((rc=pthread_create(&thread4,NULL,&encoderRigth,sharedData))){
	printf("Fallo de pthread:\n");
    }
    if ((rc=pthread_create(&thread5,NULL,&movement,sharedData))){
	printf("Fallo de pthread:\n");
    }
    
    pthread_join(thread1,NULL);
    pthread_join(thread2,NULL);
    pthread_join(thread3,NULL);
    pthread_join(thread4,NULL);
    pthread_join(thread5,NULL);
    printf("From main proces IC: %d\n",(int)getpid());
    
	destroySharedData(sharedData);
    
    return 0;
}
