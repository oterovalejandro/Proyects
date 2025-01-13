clear all
close all
% DATOS DE ENTRADA:
%Puntos de los objetos:
vertices_objeto1 = [
    6, 4;
    7, 4;
    7, 9;
    6, 9;
    6, 4
];
vertices_objeto2 = [
    10, 6;
    12, 6;
    12, 8;
    10, 8
    10, 6
];

x_obj1=vertices_objeto1(:,1)
y_obj1=vertices_objeto1(:,2)
x_obj2=vertices_objeto2(:,1)
y_obj2=vertices_objeto2(:,2)

%Se ensanchan los objetos
dist_sec = [
    -0.6, -0.6;
    0.6, -0.6;
    0.6, 0.6;
    -0.6, 0.6;
    -0.6, -0.6;
    ];

vertices_objeto2 = vertices_objeto2+dist_sec;
vertices_objeto1 = vertices_objeto1+dist_sec;

% Puntos objetivo:
p0=[13,4];
pF=[4,11]

%Tiempos de cada tramo
t1=3;
t2=2;
t3=4;

t0=0;
tD=t0+t1;
tA=t0+t1+t2;
tF=t0+t1+t2+t3;

if p0(2)<= 6 & p0(1) > 6.5 
    pD=vertices_objeto2(1,:);
elseif p0(2)<= 6 & p0(1) <= 6.5 
    pD=vertices_objeto1(2,:);
elseif p0(2)> 6 & p0(1) > 6.5
    pD=vertices_objeto2(4,:);
elseif p0(2)> 6 & p0(1) <= 6.5
    pD=vertices_objeto1(3,:);
end
if pF(2)<= 6 & pF(1) > 6.5 
    pA=vertices_objeto2(1,:);
elseif pF(2)<= 6 & pF(1) <= 6.5 
    pA=vertices_objeto1(2,:);
elseif pF(2)> 6 & pF(1) > 6.5
    pA=vertices_objeto2(4,:);
elseif pF(2)> 6 & pF(1) <= 6.5
    pA=vertices_objeto1(3,:);
end
%pA=[8.5,4];
% MATRIZ CON LAS ECUACIONES QUE HAY QUE RESOLVER:

% Se resuelve la matriz general mostrada en el ejemplo:

M=[     t0^4,   t0^3,   t0^2,   t0, 1,        0,      0,     0,   0,        0,      0,    0,  0,  0
      4*t0^3, 3*t0^2,   2*t0,   1,  0,        0,      0,     0,   0,        0,      0,    0,  0,  0
    4*3*t0^2, 3*2*t0,      2,   0,  0,        0,      0,     0,   0,        0,      0,    0,  0,  0
        tD^4,   tD^3,   tD^2,   tD, 1,    -tD^3,  -tD^2,   -tD,  -1,        0,      0,    0,  0,  0
      4*tD^3, 3*tD^2,   2*tD,   1,  0,  -3*tD^2,  -2*tD,    -1,   0,        0,      0,    0,  0,  0
    4*3*tD^2, 3*2*tD,      2,   0,  0,  -3*2*tD,     -2,     0,   0,        0,      0,    0,  0,  0
           0,      0,      0,   0,  0,    -tA^3,  -tA^2,   -tA,  -1,     tA^4,   tA^3, tA^2, tA,  1       
           0,      0,      0,   0,  0,  -3*tA^2,  -2*tA,    -1,   0,   4*tA^3, 3*tA^2, 2*tA,  1,  0
           0,      0,      0,   0,  0,  -3*2*tA,     -2,     0,   0, 4*3*tA^2, 3*2*tA,    2,  0,  0      
           0,      0,      0,   0,  0,        0,      0,     0,   0,     tF^4,   tF^3, tF^2, tF,  1
           0,      0,      0,   0,  0,        0,      0,     0,   0,   4*tF^3, 3*tF^2, 2*tF,  1,  0
           0,      0,      0,   0,  0,        0,      0,     0,   0, 4*3*tF^2, 3*2*tF,    2,  0,  0
        tD^4,   tD^3,   tD^2,   tD, 1,        0,      0,     0,   0,        0,      0,    0,  0,  0
           0,      0,      0,   0,  0,        0,      0,     0,   0,     tA^4,   tA^3, tA^2, tA,  1
     ];

Bx=[p0(1), 0, 0, 0, 0, 0, 0, 0, 0, pF(1), 0, 0, pD(1), pA(1)]';
By=[p0(2), 0, 0, 0, 0, 0, 0, 0, 0, pF(2), 0, 0, pD(2), pA(2)]';

% Ecuación:  M*pX=Bx
pX=M\Bx;
% Ecuación:  M*pY=By
pY=M\By;

% SOLUCIÓN PARA CADA VARIABLE (X,Y)

% Coeficientes de X e Y para las curvas 1 y 2:
pX1=pX(1:5)
pX2=pX(6:9)
pX3=pX(10:14)
pY1=pY(1:5)
pY2=pY(6:9)
pY3=pY(10:14)

% ----------------------------------------------------

% CÁLCULO DE LOS PUNTOS (DIBUJO):

% Cálculo de la curva, la velocidad y la aceleración:
t1=t0:0.1:tD;
t2=tD:0.1:tA;
t3=tA:0.1:tF;

% Posición:
x1=polyval(pX1,t1);
y1=polyval(pY1,t1);
x2=polyval(pX2,t2);
y2=polyval(pY2,t2);
x3=polyval(pX3,t3);
y3=polyval(pY3,t3);

% Velocidad:
dpX1=polyder(pX1);
dpY1=polyder(pY1);
dpX2=polyder(pX2);
dpY2=polyder(pY2);
dpX3=polyder(pX3);
dpY3=polyder(pY3);
dx1=polyval(dpX1,t1);
dy1=polyval(dpY1,t1);
dx2=polyval(dpX2,t2);
dy2=polyval(dpY2,t2);
dx3=polyval(dpX3,t3);
dy3=polyval(dpY3,t3);
% Aceleración:
ddpX1=polyder(dpX1);
ddpY1=polyder(dpY1);
ddpX2=polyder(dpX2);
ddpY2=polyder(dpY2);
ddpX3=polyder(dpX3);
ddpY3=polyder(dpY3);
ddx1=polyval(ddpX1,t1);
ddy1=polyval(ddpY1,t1);
ddx2=polyval(ddpX2,t2);
ddy2=polyval(ddpY2,t2);
ddx3=polyval(ddpX3,t3);
ddy3=polyval(ddpY3,t3);

% Dibujo de la solución:

figure
xlim([0, 13]);
ylim([0, 12]);
plot(x1,y1,x2,y2,x3,y3,p0(1),p0(2),'o',pD(1),pD(2),'o',pA(1),pA(2),'o',pF(1),pF(2),'o')
hold on;
plot(x_obj1, y_obj1, 'b', 'LineWidth', 2);
plot(x_obj2, y_obj2, 'b', 'LineWidth', 2);
x_obj1=vertices_objeto1(:,1)
y_obj1=vertices_objeto1(:,2)
x_obj2=vertices_objeto2(:,1)
y_obj2=vertices_objeto2(:,2)
plot(x_obj1, y_obj1, 'r--');
plot(x_obj2, y_obj2, 'r--');
title('Curvas 1 (azul), 2 (naranja) y 3 (amarillo)');
hold off
figure
subplot(3,2,1), plot(t1,x1,'r',t2,x2,'b',t3,x3,'g'),title('x(t)')
subplot(3,2,2), plot(t1,y1,'r',t2,y2,'b',t3,y3,'g'),title('y(t)')
subplot(3,2,3), plot(t1,dx1,'r',t2,dx2,'b',t3,dx3,'g'),title('dx(t)')
subplot(3,2,4), plot(t1,dy1,'r',t2,dy2,'b',t3,dy3,'g'),title('dy(t)')
subplot(3,2,5), plot(t1,ddx1,'r',t2,ddx2,'b',t3,ddx3,'g'),title('ddx(t)')
subplot(3,2,6), plot(t1,ddy1,'r',t2,ddy2,'b',t3,ddy3,'g'),title('ddy(t)')

%------------------------------------------------------------
% Parámetros del problema
R=0.25; % Radio de las ruedas (m)
d=1;  % Separación entre las ruedas (m)

x_inicial=p0(1);
y_inicial=p0(2);
phi_inicial=0;

% Seguimiento:
Lref=0.5;

x = [x1,x2,x3];
y = [y1,y2,y3];
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

figure
plot(tout,w)
xlabel('s');ylabel('radianes/s'); title('Acción de control del vehículo')
figure
plot(tout,alpha)
xlabel('s');ylabel('radianes'); title('Acción de control del vehículo')
