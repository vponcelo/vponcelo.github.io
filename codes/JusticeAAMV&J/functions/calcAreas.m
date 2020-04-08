function [intersect_U,h1,h2] = calcAreas(b,b_ant)
% Calculate the areas of both the intersection between the current and
% previous AAM and the current AAM bounding box

%% Input
% Current and previous AAM representation
%% Output
% a_intersection: Area of the intersection between AAM's bounding boxes
% a_box: Area of the current AAM bounding box
% a_box_ant: Area of the previous AAM bounding box

x = (b.xy(:,1)+b.xy(:,3))/2;
y = (b.xy(:,2)+b.xy(:,4))/2;
x_min = min(x); x_max = max(x); y_min = min(y); y_max = max(y);
w = dist(x_max,x_min); h = dist(y_max,y_min);
box1 = [x_min y_min w h];
h1 = rectangle('Position',[x_min,y_min,w,h],'LineWidth',2,'EdgeColor','g');

a_box = w*h;

x_ant = (b_ant.xy(:,1)+b_ant.xy(:,3))/2;
y_ant = (b_ant.xy(:,2)+b_ant.xy(:,4))/2;
x_min_ant = min(x_ant); x_max_ant = max(x_ant); 
y_min_ant = min(y_ant); y_max_ant = max(y_ant);
w = dist(x_max_ant,x_min_ant); h = dist(y_max_ant,y_min_ant);
box2 = [x_min_ant y_min_ant w h];
h2 = rectangle('Position',[x_min_ant,y_min_ant,w,h],'LineWidth',2,'EdgeColor','y');

a_box_ant = w*h;
intersect = rectint(box1,box2);
intersect_U = intersect/(a_box + a_box_ant - intersect);
