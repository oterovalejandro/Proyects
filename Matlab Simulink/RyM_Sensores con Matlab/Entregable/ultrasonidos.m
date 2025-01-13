function [MapOut,h]=ultrasonidos(Map, sensor, rob, beta)
% Finding if there are objects round the robot
% 
% [MapOut,h]=ultrasonidos(Map, sensor, rob, beta),
% 
% MapOut    Updated sensed Map
% h         Handle of objects when DEBUG is used (otherwise empty)
%
% Map       Map.M, Real Map / Map.sen, Sensed Map
% sensor    Sensor model
% rob       Robot model
% beta      Orient of the sonar sensor (in relation to course robot)
%
global DEBUG

posX=rob.x(end);
posY=rob.y(end);
orientRobot=rob.theta(end);

MapOut=Map.sen;
MapaReal=Map.M;

h=[];

valtheta=(beta+orientRobot)-sensor.rangTetha/2:0.05:(beta+orientRobot)+sensor.rangTetha/2;
valrange=0:0.1:sensor.rangR;

if (DEBUG)
    ptoArcI=[posX+sensor.rangR*cos(valtheta(1));posY-sensor.rangR*sin(valtheta(1))]';
    ptoArcF=[posX+sensor.rangR*cos(valtheta(end));posY-sensor.rangR*sin(valtheta(end))]';
    Arc=[posX+sensor.rangR*cos(valtheta);posY-sensor.rangR*sin(valtheta)]';
    figure(2),          
    h(1)=line([posX, ptoArcI(1)], [posY, ptoArcI(2)]);
    h(2)=line([posX, ptoArcF(1)], [posY, ptoArcF(2)]);
    h(3)=plot(Arc(:,1),Arc(:,2),'.');
end

% Simulate sensor:
% Look for an object into ultrasonic sensor section (find the index of real
% Map which they are into the sensor section)  
% Update these index in the sensed Map: use the model sensor and 
% Bayesian fusion 


% ***************** Example: ****************************
% This is a simple example in order to show how paint in the Map:
%       I suppose there aren´t objetcs from of the robot because I  
%       don't look for in the real Map. For example, I update 
%       the sensed map cells to 0.2

Mr=length(valrange);
[f,c]=size(Map.M);

r0=sensor.rangR+sensor.epsil;
for k=linspace(Mr,1,Mr)
    ptoArc=[posX+valrange(k)*cos(valtheta);posY-valrange(k)*sin(valtheta)]';
    for a=1:length(valtheta)
        if (MapaReal(floor(ptoArc(a,1))*c+floor(ptoArc(a,2)))==1)
            r0=sqrt((ptoArc(a,1)-posX)^2+(-ptoArc(a,2)+posY)^2)
        end
    end
end
for k=1:Mr
    ptoArc=[posX+valrange(k)*cos(valtheta);posY-valrange(k)*sin(valtheta)]';
    for i=1:length(valtheta)
       if (sqrt((ptoArc(i,1)-posX)^2+(ptoArc(i,2)-posY)^2)<r0)
        MapOut(floor(ptoArc(i,1))*c+floor(ptoArc(i,2)))=(MapOut(floor(ptoArc(i,1))*c+floor(ptoArc(i,2)))*0.1)   /((MapOut(floor(ptoArc(i,1))*c+floor(ptoArc(i,2)))*0.1)+(1-MapOut(floor(ptoArc(i,1))*c+floor(ptoArc(i,2))))*(1-0.1));
       elseif(r0<=sqrt((ptoArc(i,1)-posX)^2+(ptoArc(i,2)-posY)^2))&&(sqrt((ptoArc(i,1)-posX)^2+(ptoArc(i,2)-posY)^2)<=r0+sensor.epsil)
        MapOut(floor(ptoArc(i,1))*c+floor(ptoArc(i,2)))=(MapOut(floor(ptoArc(i,1))*c+floor(ptoArc(i,2)))*(0.5+0.5/sensor.kd))/((MapOut(floor(ptoArc(i,1))*c+floor(ptoArc(i,2)))*(0.5+0.5/sensor.kd))+(1-MapOut(floor(ptoArc(i,1))*c+floor(ptoArc(i,2))))*(1-(0.5+0.5/sensor.kd)));
       else
        MapOut(floor(ptoArc(i,1))*c+floor(ptoArc(i,2)))=(MapOut(floor(ptoArc(i,1))*c+floor(ptoArc(i,2)))*0.5)/((MapOut(floor(ptoArc(i,1))*c+floor(ptoArc(i,2)))*0.5)+(1-MapOut(floor(ptoArc(i,1))*c+floor(ptoArc(i,2))))*(1-0.5));
       end
    end
end
% *********************************************************

