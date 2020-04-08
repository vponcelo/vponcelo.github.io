%% Read and a text file.
path = 'C:/victor/thesis/datasets/Justicia/2012-09-19_Barcelona/r17_s1/denunciat_traductora/Copyp1.txt';
f_id=fopen(path,'r');
reports=textscan(f_id, '%s', 'Delimiter', '\n');
fclose(f_id);
reports_saved=reports{1};
n_lines = length(reports_saved);

%% Write a new modified text file.
path = 'C:/victor/thesis/datasets/Justicia/2012-09-19_Barcelona/r17_s1/denunciat_traductora/p1_faceFeatures.txt';
f_id=fopen(path,'w');
fileText = char();
for i = 1:n_lines,
    parts = regexp(reports_saved{i},'\t','split');
    
    if i == 1, 
        [x,y,z,x_ant,y_ant,z_ant,d_x,d_y,d_z,dabs,dd_x,dd_y,dd_z,ddabs,...
        d_x_ant,d_y_ant,d_z_ant] = getDataFromText(parts,i);
    else 
        [x,y,z,x_ant,y_ant,z_ant,d_x,d_y,d_z,dabs,dd_x,dd_y,dd_z,ddabs,...
        d_x_ant,d_y_ant,d_z_ant] = getDataFromText(parts,i,x_ant,y_ant,...
        z_ant,d_x_ant,d_y_ant,d_z_ant);
    end
    
    if ischar(dabs) && ischar(ddabs),
        text = sprintf('\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f',x,y,z,...
            x_ant,y_ant,z_ant);
    elseif ischar(dabs),
        text = sprintf('\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f',...
            x,y,z,x_ant,y_ant,z_ant,d_x,d_y,d_z,dabs,d_x_ant,d_y_ant,...
            d_z_ant);
    else
        text = sprintf('\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f',...
            x,y,z,x_ant,y_ant,z_ant,d_x,d_y,d_z,dabs,d_x_ant,d_y_ant,...
            d_z_ant,dd_x,dd_y,dd_z,ddabs);
    end
    
%     [alpha0,alpha1,alpha2] = getAnglePeople(reports_saved{i});
%     text = sprintf('\t%.2f\t%.2f\t%.2f',a,b,c);
    text = strcat(reports_saved{i},text);
    fileText = sprintf('%s%s\n',fileText,text);
end
fprintf(f_id,'%s',fileText);
fclose(f_id);