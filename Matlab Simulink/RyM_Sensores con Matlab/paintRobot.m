function vRobotMap=paintRobot(Robot,tam)
%
% Get points of robot in order to draw it using the fill fuction 
%
% vRobotMap=paintRobot(Robot,tam);
%
% RobotXY:   (Robot.x,Robot.y), center of robot (cm)
%                               x are columns, y are rows.
%            Robot.theta,  Course of robot (to x) (rad)
%
% tam:       size of robot [length,width]

posX=Robot.x(end);
posY=Robot.y(end);
thetaRobot=Robot.theta(end)+pi/2;


ROT =[ cos(thetaRobot)  sin(thetaRobot)  0  posX
      -sin(thetaRobot)  cos(thetaRobot)  0  posY
       0                  0              1    0 
       0                  0              0    1];
  
VBody=       [-tam(1)/2  tam(2)/2 0 1
               tam(1)/2  tam(2)/2 0 1
               tam(1)/2 -tam(2)/2 0 1
              -tam(1)/2 -tam(2)/2 0 1]';
          
vRobotMap=ROT*VBody;
