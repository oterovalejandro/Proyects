% Limpiar el espacio de trabajo
clear all
close all

% Definir variable global de depuración
global DEBUG

% Activar o desactivar depuración (0 para desactivar, 1 para activar)
DEBUG = 0;

% ================================================================
% INPUTS:

% --------- MAP --------------------------------------------------
% Leer un mapa real desde un archivo
MapReal = load('Mapa.mat'); % Objetos = 1, vacío = 0

% Mapa sentido por el robot:
Mapa = 0.5 * ones(size(MapReal.M)); % Desconocido = 0.5
% ----------------------------------------------------------------

% --------- Model Robot ------------------------------------------
% Parámetros del robot
rob.Nc = 100;                   % Pulsos totales del encoder
rob.Rb = 5;                     % Radio de la rueda (cm)
rob.b = 10;                     % Distancia entre ruedas (cm)
rob.x(1) = 200; rob.y(1) = 100; % Posición inicial del robot
rob.theta(1) = 0;               % Orientación inicial del robot (rad)
% ----------------------------------------------------------------

% --------- Model Sensor data ------------------------------------
sensor.rangTetha = 30 * pi / 180; % Ángulo del haz del sensor (rad)
sensor.rangR = 80;                % Alcance del sensor (cm)
sensor.epsil = 10;                % Error de medición del sensor (cm)
sensor.kd = 3;                    % Parámetro kd del sensor
% ----------------------------------------------------------------

% --------- Encoder readings ------------------------------------
% Lecturas de los encoders cuando el robot se mueve
Enc = load('Encoder.dat');
% ----------------------------------------------------------------

% Dibujar el mapa inicial del robot
figure(1)
image(Mapa, 'CDataMapping', 'scaled');
title('Mapa Detectado');
xlabel('Eje x');
ylabel('Eje y');
caxis([0 1]);
colorbar;
hold on;

if (DEBUG)
    % Dibujar el mapa real para depuración
    figure(2)
    image(MapReal.M, 'CDataMapping', 'scaled');
    title('Mapa Real');
    xlabel('Eje x');
    ylabel('Eje y');
    caxis([0 1]);
    colorbar;
    hold on;  
end;

% Dibujar el robot
figure(1)
V = paintRobot(rob, [10, 20]);
colorRob = [0 0 0];
r = fill(V(1,:), V(2,:), colorRob);

if (DEBUG)
    figure(2)
    r2 = fill(V(1,:), V(2,:), colorRob);
end;

disp('Presione cualquier tecla');
pause;

% ===================================================================
% Simulación y trazado:

for i = 1:length(Enc.Nl)
    
    % Mover el robot según el modelo de movimiento
    D = 3; % Distancia de movimiento (fija en este ejemplo)
    dtheta = 0; % Cambio de orientación (fijo en este ejemplo)
    rob.x(i+1) = rob.x(i) + D * cos(rob.theta(i));
    rob.y(i+1) = rob.y(i) - D * sin(rob.theta(i));
    rob.theta(i+1) = rob.theta(i) + dtheta;

    % Escanear alrededor del robot
    for k = 1:3 % Realizar 3 medidas
        [M, ~] = ultrasonidos(MapReal, sensor, rob, 0); % Ángulo fijo para adelante
        Mapa = M;
    end

    % Dibujar el mapa sentido
    figure(1)
    image(Mapa, 'CDataMapping', 'scaled');

    % Dibujar el robot en su nueva posición
    figure(1)
    delete(r); % Eliminar el robot anterior
    V = paintRobot(rob, [10, 20]);
    colorRob = [0.6 0.6 0.6];
    r = fill(V(1,:), V(2,:), colorRob);

    if (DEBUG)
        % Dibujar la posición del robot en el mapa real para depuración
        figure(2)
        set(r2, 'Visible', 'off')
        r2 = fill(V(1,:), V(2,:), colorRob);
    end;

    pause(0.1)
end
