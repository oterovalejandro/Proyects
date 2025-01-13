% Parámetros del Robot diferencial 
% Nota: El ejemplo entregado no tiene un robot diferencial

%------------------------------------------------------------
% Parámetros del problema
R=0.25; % Radio de las ruedas (m)
d=5;  % Separación entre las ruedas (m)

x_inicial=2;
y_inicial=1;
phi_inicial=0;

% Seguimiento:
Lref=0.5;

%Camino (linea recta):
% Definir el radio del círculo
radio = 5;
% Definir el ángulo theta (en radianes)
theta = linspace(-pi, pi, 1000); % 100 puntos para suavizar el círculo

% Calcular las coordenadas x e y del círculo
x = 6 + radio * cos(theta);
y = 7 + radio * sin(theta);

%Calcular las coordenadas de una sinusoide
%x = theta;
%y = 3*sin(theta*2*pi/6);
%------------------------------------------------------------
% Ejecutar desde línea de comandos el modelo/simulación de Simulink
sim('simulacionRobot.slx')

%------------------------------------------------------------
% Realización de figuras para estudiar el resultado:
figure
plot(x,y,x_v,y_v)
xlabel('m'); ylabel('m'); title('Camino seguido')
figure
phi_v = mod(phi_v,2*pi)-pi;
plot(tout,phi_v)
xlabel('s');ylabel('radianes'); title('Orientación del vehículo')

