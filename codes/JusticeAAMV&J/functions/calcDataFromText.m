function [x_ant,y_ant,z_ant,d_x,d_y,d_z,dabs,dd_x,dd_y,dd_z,ddabs,...
    d_x_ant,d_y_ant,d_z_ant] = calcDataFromText(parts,i,x_ant,y_ant,...
    z_ant,d_x_ant,d_y_ant,d_z_ant)

x = parts(2); x = str2double(x{1});
y = parts(3); y = str2double(y{1});
z = parts(4); z = str2double(z{1});
d_x = ''; d_y = ''; d_z = ''; dd_x = ''; dd_y = ''; dd_z = '';
dabs = ''; ddabs = '';

if i > 1 && exist('x_ant','var'),
    d_x = x - x_ant; d_y = y - y_ant; d_z = z - z_ant;
    dabs = sqrt(d_x^2+d_y^2+d_z^2);
    if i > 2,
        dd_x = d_x - d_x_ant; dd_y = d_y - d_y_ant; dd_z = d_z - d_z_ant;
        ddabs = sqrt(dd_x^2+dd_y^2+dd_z^2);
    end
    d_x_ant = d_x; d_y_ant = d_y; d_z_ant = d_z;        
else
    d_x_ant = ''; d_y_ant = ''; d_z_ant = '';
end

x_ant = x; y_ant = y; z_ant = z; 
