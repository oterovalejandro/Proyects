#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

float h = 1e-4;

typedef struct {
    float x;
    float y;
    float z;
    double *surfaceCoefficients;
    int dimension;
} Point3D;

typedef struct {
    double dxx;
    double dxy;
    double dyy;
} TensorCurvatura;

void evaluate(Point3D *r){
    r->z = 0.0;
    for (int i = 0; i < r->dimension; i++) {
        for (int j = 0; j < r->dimension; j++) {
            r->z += (r->surfaceCoefficients[i*r->dimension+j])*pow(r->x, i)*pow(r->y, j);
        }
    }
}


Point3D findIntersection(float u, float v, Point3D *function, Point3D origin) {
    float direction_x = tan(u);
    float direction_y = tan(v);
    float direction_z = 1;

    Point3D intersectionPoint;
    float z;
    float t=-fmax(abs((-30.0-origin.x)/direction_x),abs((-30.0-origin.y)/direction_y));

    do {
        function->x = origin.x + direction_x * t;
        function->y = origin.y + direction_y * t;
        z = origin.z + direction_z * t;

        evaluate(function);
        t+=h;

    } while (fabs(function->z - z) > h);

    intersectionPoint.x = function->x;
    intersectionPoint.y = function->y;
    intersectionPoint.z = z;
    return intersectionPoint;
}

double second_derivative_xx(Point3D *r0) {
    double dxx = 0;
    for (int i = 0; i < r0->dimension; i++) {
        for (int j = 0; j < r0->dimension; j++) {
            if (i >= 2) {
                dxx += r0->surfaceCoefficients[i*r0->dimension+j] * i * (i-1) * pow(r0->x, i-2) * pow(r0->y, j);
            }
        }
    }
    //printf("dxx: %lf\n",dxx);
    return dxx;
}

double second_derivative_yy(Point3D *r0) {
    double dyy = 0;
    for (int i = 0; i < r0->dimension; i++) {
        for (int j = 0; j < r0->dimension; j++) {
            if (j >= 2) {
                dyy += r0->surfaceCoefficients[i*r0->dimension+j] * j * (j-1) * pow(r0->x, i) * pow(r0->y, j-2);
            }
        }
    }
    //printf("dyy: %lf\n",dyy);
    return dyy;
}

double second_derivative_xy(Point3D *r0) {
    double dxy = 0;
    for (int i = 0; i < r0->dimension; i++) {
        for (int j = 0; j < r0->dimension; j++) {
            if (j >= 1 && i>= 1) {
                dxy += r0->surfaceCoefficients[i * r0->dimension + j] * j * i * pow(r0->x, i - 1) * pow(r0->y, j - 1);
            }
        }
    }
    //printf("dxy: %lf\n",dxy);
    return dxy;
}

TensorCurvatura findCurvature(Point3D *r0) {
    TensorCurvatura curvatureCoefficients;
    curvatureCoefficients.dxx = second_derivative_xx(r0);
    curvatureCoefficients.dxy = second_derivative_xy(r0);
    curvatureCoefficients.dyy = second_derivative_yy(r0);
    return curvatureCoefficients;
}

void mainCurvatures(Point3D *r0, double *curvaturasPrincipales) {
    TensorCurvatura curvatureCoefficients = findCurvature(r0);

    double discriminante = curvatureCoefficients.dxx * curvatureCoefficients.dyy - curvatureCoefficients.dxy * curvatureCoefficients.dxy;
    double traza = curvatureCoefficients.dxx + curvatureCoefficients.dyy;

    curvaturasPrincipales[0] = (traza + sqrtl(traza * traza - 4 * discriminante)) / 2;
    curvaturasPrincipales[1] = (traza - sqrtl(traza * traza - 4 * discriminante)) / 2;
}

void chargeValue(Point3D *r){
    FILE *archivo;
    char buffer[100];
    r->dimension = 0;

    archivo = fopen("sup.txt", "r");

    if (archivo == NULL) {
        printf("No se pudo abrir el archivo.\n");
        exit(EXIT_FAILURE);
    }

    if (fgets(buffer, sizeof(buffer), archivo)) {
        char *ptr = buffer;
        while (*ptr != '\0') {
            while (isspace(*ptr)) {
                ptr++;
            }
            if (isdigit(*ptr)) {
                r->dimension++;
                while (*ptr != '\0' && !isspace(*ptr)){ptr++;}
            } else {
                ptr++;
            }
        }
    }
    printf("%d\n",r->dimension);
    fseek(archivo, 0, SEEK_SET);

    r->surfaceCoefficients = (double *)malloc(pow(r->dimension,2) * sizeof(double));

    for (int i = 0; i < pow(r->dimension,2); i++) {
        if (fscanf(archivo, "%lf", &r->surfaceCoefficients[i]) != 1) {
            printf("Error al leer los datos del archivo.\n");
            fclose(archivo);
            free(r->surfaceCoefficients);
            exit(EXIT_FAILURE);
        }
    }

    fclose(archivo);

    /*
    printf("Datos almacenados en el array de dos dimensiones:\n");
    for (int i = 0; i < pow(r->dimension,2); i++) {
            printf("%lf ", r->surfaceCoefficients[i]);
    }
    printf("\n");*/
}

Point3D *definePoint3D(){
    Point3D *r = malloc(sizeof(Point3D));
    if (r == NULL) {
        printf("Error: No se pudo asignar memoria para Point3D.\n");
        exit(EXIT_FAILURE);
    }
    chargeValue(r);
    r->x = 0.0;
    r->y = 0.0;
    r->z = 0.0;
    return r;
}

void modifyCoordinates(float x, float y, float z, Point3D *r){
    r->x = x;
    r->y = y;
    r->z = z;
}

void printCoordinates(Point3D *function){
    /*printf("Datos almacenados en el array de dos dimensiones:\n");
    for (int i = 0; i < function->dimension*function->dimension; i++) {
            printf("%lf ", function->surfaceCoefficients[i]);
            //function->surfaceCoefficients++;
    }
    printf("\n");*/
    printf("r = (%lf, %lf, %lf)\tz(%lf, %lf) = %lf\n",function->x,function->y,function->z,function->x,function->y,function->z);

}

int main() {
    Point3D *function = definePoint3D();

    /* Ejercicio1.- Evaluación de z(x,y) en distintos puntos.*/
    printf("r = (x, y, z)\n");

    modifyCoordinates(-10,5,0,function);
    evaluate(function);
    printCoordinates(function);

    modifyCoordinates(5,10,0,function);
    evaluate(function);
    printCoordinates(function);

    modifyCoordinates(10,10,0,function);
    evaluate(function);
    printCoordinates(function);

    /*Ejercicio2.- Cálculo de intersecciones*/
    Point3D origin;
    origin.x = 0;
    origin.y = 0;
    origin.z = 20;


    float u,v;

    u = -20*M_PI/180;
    v = 10*M_PI/180;
    Point3D intersectionPoint = findIntersection(u, v, function, origin);
    // Imprime el punto de intersección
    printf("Punto de intersección para los ángulos (u: %d, v: %d), r = (%f, %f, %f)\n", -20, 10, intersectionPoint.x, intersectionPoint.y, intersectionPoint.z);
    printf("En ese punto z (x, y) = %f\n",function->z);
    double curvatures[2];

    u = 10*M_PI/180;
    v = 20*M_PI/180;
    intersectionPoint = findIntersection(u, v, function, origin);
    // Imprime el punto de intersección
    printf("Punto de intersección para los ángulos (u: %d, v: %d), r = (%f, %f, %f)\n", 10, 20, intersectionPoint.x, intersectionPoint.y, intersectionPoint.z);
    printf("En ese punto z (x, y) = %f\n",function->z);

    u = -15*M_PI/180;
    v = -15*M_PI/180;
    intersectionPoint = findIntersection(u, v, function, origin);
    // Imprime el punto de intersección
    printf("Punto de intersección para los ángulos (u: %d, v: %d), r = (%f, %f, %f)\n", -15, -15, intersectionPoint.x, intersectionPoint.y, intersectionPoint.z);
    printf("En ese punto z (x, y) = %f\n",function->z);

    /* Ejercicio3.- Cálculo de curvaturas principales de la superficie*/
    modifyCoordinates(10,5,0, function);
    mainCurvatures(function,&curvatures[0]);
    printf("Curvatura principal k1 en el punto (10, 5), %f\n", curvatures[0]);
    printf("Curvatura principal k2 en el punto (10, 5), %f\n", curvatures[1]);


    modifyCoordinates(10,-10,0, function);
    mainCurvatures(function,&curvatures[0]);
    printf("Curvatura principal k1 en el punto (10, -10), %f\n", curvatures[0]);
    printf("Curvatura principal k2 en el punto (10, -10), %f\n", curvatures[1]);


    free(function->surfaceCoefficients);
    free(function);
    return 0;
}

/* Función para calcular la segunda derivada parcial de f respecto a x utilizando diferencias finitas de cinco puntos
double second_derivative_xx(float x, float y, double *surfaceCoefficients, int dimension) {
    // Diferencia finita de cinco puntos para la segunda derivada parcial respecto a x
     double result = (-evaluate(x - 2 * h, y, surfaceCoefficients, dimension) + 16 * evaluate(x - h, y, surfaceCoefficients, dimension) - 30 * evaluate(x, y, surfaceCoefficients, dimension) + 16 * evaluate(x + h, y, surfaceCoefficients, dimension) - evaluate(x + 2 * h, y, surfaceCoefficients, dimension)) / (12 * h * h);
    printf("Segunda derivada parcial de f respecto a x: %Lf\n", result);
    return result;
}

// Función para calcular la segunda derivada parcial de f respecto a y utilizando diferencias finitas de cinco puntos
double second_derivative_yy(float x, float y, double *surfaceCoefficients, int dimension) {
    // Diferencia finita de cinco puntos para la segunda derivada parcial respecto a y
     double result = (-evaluate(x, y - 2 * h, surfaceCoefficients, dimension) + 16 * evaluate(x, y - h, surfaceCoefficients, dimension) - 30 * evaluate(x, y, surfaceCoefficients, dimension) + 16 * evaluate(x, y + h, surfaceCoefficients, dimension) - evaluate(x, y + 2 * h, surfaceCoefficients, dimension)) / (12 * h * h);
    printf("Segunda derivada parcial de f respecto a y: %Lf\n", result);
    return result;
}

// Función para calcular la segunda derivada parcial cruzada de f respecto a x e y utilizando diferencias finitas de cinco puntos
double second_derivative_xy(float x, float y, double *surfaceCoefficients, int dimension) {
    // Diferencia finita de cinco puntos para la segunda derivada parcial cruzada
     double result = (evaluate(x - h, y - h, surfaceCoefficients, dimension) - 8 * evaluate(x - h, y + h, surfaceCoefficients, dimension) + 8 * evaluate(x + h, y - h, surfaceCoefficients, dimension) - evaluate(x + h, y + h, surfaceCoefficients, dimension)) / (12 * h * h);
    printf("Segunda derivada parcial cruzada de f respecto a x e y: %Lf\n", result);
    return result;
}*/