clear all
close all

global DEBUG

DEBUG=1;

% ================================================================
% INPUTS:

% --------- MAP --------------------------------------------------
%Read a real map
    Map=load('Mapa.mat'); % Objects = 1, empty 0
% Sensed map:
    Map.sen=0.5*ones(size(Map.M)); % unknown 0.5
% ----------------------------------------------------------------

% --------- Model Robot ------------------------------------------
rob.Nc=100;                 % Encoder: total pulses 
rob.Rb=5;                   % wheel radius (cm)
rob.b=10;                   % Distance between wheels (cm)
rob.x(1)=200; rob.y(1)=400; % Position of robot
rob.theta(1)=0;             % Orient of robot
% ----------------------------------------------------------------

% --------- Model Sensor data ------------------------------------
sensor.rangTetha=30*pi/180; % Beam: Width (rad)
sensor.rangR=80;            % Beam: Range (cm)
sensor.epsil=10;            % error (cm)
sensor.kd=3;
% ----------------------------------------------------------------

% --------- Encoder readings ------------------------------------
% The robot get encoders readings when it moves 
% Readings of encoder sensors (Nl, letf encoder, Nr right encoder):
E=load('Encoder.dat');
Enc.Nl=E(:,1);
Enc.Nr=E(:,2);
% ----------------------------------------------------------------

% Draw the initial map of the robot:
figure(1)
image(Map.sen,'CDataMapping','scaled'); 
title('Mapa Detectado');
xlabel('Eje x');
ylabel('Eje y');
clim([0 1]);
colorbar;
hold on;

if (DEBUG)
    % Draw real Map in order to Debug
    figure(2)
    image(Map.M,'CDataMapping','scaled');
    title('Mapa Real');
    xlabel('Eje x');
    ylabel('Eje y');
    clim([0 1]);
    colorbar;
    hold on;  
end

% Draw the robot
figure(1)
V=paintRobot(rob,[10,20]);
colorRob=[0 0 0];
r=fill(V(1,:),V(2,:),colorRob);

if (DEBUG)
    figure(2)
    r2=fill(V(1,:),V(2,:),colorRob);
end

disp('Press any key');
pause;

% ===================================================================
% Simulation and plot:

for i=1:length(Enc.Nl)
    
% ----- Move robot (motion model) ----------    

% Get next robot position using Nl y Nr. 

    % ***************** Example:  ****************************
    %   In this case I move a fixed length D=3 cm.
    %   and the same course (initial orientation of robot)

        rob.x(i+1)=rob.x(i)+(Enc.Nl(i)+Enc.Nr(i))*(pi/20)*cos(rob.theta(i));
        rob.y(i+1)=rob.y(i)-(Enc.Nl(i)+Enc.Nr(i))*(pi/20)*sin(rob.theta(i));  % Warning: we use "-" because y axis is reverse en the plot
        rob.theta(i+1)=rob.theta(i)+(Enc.Nr(i)-Enc.Nl(i))*pi/100;

    % *********************************************************

% ------ Scan round robot --------------

%thetaScan=0:30:360; % sonar moves around the robot 
thetaScan=0;         % sonar fixed to front robot 

for k=1:3
    [M,h]=ultrasonidos(Map, sensor, rob,thetaScan);
    Map.sen=M;
    if (DEBUG)
          s=sprintf('Rob(%f,%f,%f) ',rob.x(i+1),rob.y(i+1),rob.theta(i+1));
          disp(s);
    end
end % thetaScan

% ------- Draw sensed Map --------------
figure(1)
image(Map.sen,'CDataMapping','scaled'); 

% ------- Draw Robot -------------------
figure(1)
%r=fill(V(1,:),V(2,:),[0 0 0]); 
%set(r,'Visible','off') 
delete(r); % Clear the last robot 
% Draw new robot 
V=paintRobot(rob,[10,20]);
colorRob=[0.6 0.6 0.6];
r=fill(V(1,:),V(2,:),colorRob); 
% --------------------------------------

if (DEBUG)
    % Plot position of robot in real Map: 
    %   you can plot any data/info in order to help you
    figure(2)
    set(r2,'Visible','off')
    r2=fill(V(1,:),V(2,:),colorRob);
    set(h(1),'Visible','off')
    set(h(2),'Visible','off')
    set(h(3),'Visible','off')
end


pause(0.1)

end

% ===================================================================

