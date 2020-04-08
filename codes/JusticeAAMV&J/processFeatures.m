% Overwrite or create modified text files iteratively

root = 'C:/victor/thesis/datasets/Justicia/2012-09-14_Barcelona\r15_s1\denunciat/';
path = strcat(root,'p1.txt');
path1 = strcat(root,'p1_faceFeatures1.txt');
path2 = strcat(root,'p1_faceFeatures2.txt');
path3 = strcat(root,'p1_faceFeatures3.txt');
finalPath = strcat(root,'r15s1_F1.dat');


%% Read first feature textfile.

% f_id=fopen(path,'r');
% reports=textscan(f_id, '%s', 'Delimiter', '\n');
% fclose(f_id);
% reports_saved=reports{1};
% n_lines = length(reports_saved);
% 

%% Write with replaced Manual Corrections of the end of textfile.
% i = 1; NUM_END_LINE = 13556;
% NUM_CORRECTIONS_FIRST_LINE = 13559;
% j = NUM_CORRECTIONS_FIRST_LINE;
% 
% while j <= n_lines,
%     found = false;
%     parts1 = regexp(reports_saved{j},'\t','split');
%     while i < NUM_END_LINE && ~found,
%         parts2 = regexp(reports_saved{i},'\t','split');
%         if strcmp(parts1(1),parts2(1)),
%             found = true;
%             reports_saved(i) = reports_saved(j);
%             reports_saved(j) = [];            
%         end
%         i = i + 1;
%     end
%     if n_lines > length(reports_saved),
%         n_lines = length(reports_saved);
%     else
%         j = j + 1;
%     end
% end
% 
% n_lines = length(reports_saved);
% 
% if ~exist(path1,'file'),
%     fid = fopen(path, 'w');
%     for i = 1:n_lines,
%         fprintf(fid,sprintf('%s\n',reports_saved{i}));
%     end
%     fclose(fid);
% end



%% Replace lines with too large number of columns

% for i = 1:n_lines,   
%     parts = regexp(reports_saved{i},'\t','split');
%     if length(parts) == 7 || length(parts) == 8;
%         if length(parts) > 7,
%             parts(7) = [];                
%         end
%         parts{6} = parts{6}(1);
%         text = char(cell2mat(parts(1)));
%         for j = 2:length(parts), 
%             text = sprintf('%s\t%s',text,parts{j});
%         end    
%         reports_saved{i} = text;        
%     end    
% end




%% Replace an incorrect range by -1 values.
% copyfile(path,path1);

% f_id=fopen(path1,'r');
% reports=textscan(f_id, '%s', 'Delimiter', '\n');
% fclose(f_id);
% reports_saved=reports{1};
% n_lines = length(reports_saved);
% 
% for i = 11618:11665,
%     parts = regexp(reports_saved{i},'\t','split');
%     text = parts{1};
%     text = sprintf('%s\t%s\t%s\t%s\t-1\t2\t1.00',text,parts{2},parts{3},parts{4});              
% %     text = sprintf('%s\t-1.00\t-1.00\t-1.00\t-1\t2\t1.00',text); 
%     reports_saved{i} = text;        
% end
% 
% fid = fopen(path1, 'r+');
% for i = 1:n_lines,
%     fprintf(fid,sprintf('%s\n',reports_saved{i}));
% end
% fclose(fid);



%% Write with interpolated values.

% f_id=fopen(path1,'r');
% reports=textscan(f_id, '%s', 'Delimiter', '\n');
% fclose(f_id);
% reports_saved=reports{1};
% n_lines = length(reports_saved);
% 
% count = 0; count2 = 0;
% 
% for i = 1:n_lines,
%     parts = regexp(reports_saved{i},'\t','split');    
%     x = str2double(parts{2});
%     y = str2double(parts{3});
%     z = str2double(parts{4});
%     alpha = str2double(parts{5});
%     if x == -1.00 && y == -1.00 && z == -1.00 && i < n_lines,
%         count = count + 1;      
%     elseif count > 0 && (x ~= -1.00 && y ~= -1.00 && z ~= -1.00 || i == n_lines),
%         reports_saved = interpolateNoise(reports_saved,count,i,x,2);
%         reports_saved = interpolateNoise(reports_saved,count,i,y,3);
%         reports_saved = interpolateNoise(reports_saved,count,i,z,4);
%         count = 0;
%     end
%     if alpha == -1 && i < n_lines, 
%         count2 = count2 + 1;
%     elseif count2 > 0 && (alpha ~= -1 || i == n_lines),
%         reports_saved = interpolateNoise(reports_saved,count2,i,alpha,5);
%         count2 = 0;
%     end
% end
% 
% fid = fopen(path2, 'w');
% for i = 1:n_lines,
%     fprintf(fid,sprintf('%s\n',reports_saved{i}));
% end
% fclose(fid);




%% Write adding the rest of calculated features.

% f_id=fopen(path2,'r');
% reports=textscan(f_id, '%s', 'Delimiter', '\n');
% fclose(f_id);
% reports_saved=reports{1};
% n_lines = length(reports_saved);
% 
% f_id=fopen(path3,'w');
% fileText = char();
% for i = 1:n_lines,
%     parts = regexp(reports_saved{i},'\t','split');
%     
%     if i == 1, 
%         [x_ant,y_ant,z_ant,d_x,d_y,d_z,dabs,dd_x,dd_y,dd_z,ddabs,...
%         d_x_ant,d_y_ant,d_z_ant] = calcDataFromText(parts,i);
%     else 
%         [x_ant,y_ant,z_ant,d_x,d_y,d_z,dabs,dd_x,dd_y,dd_z,ddabs,...
%         d_x_ant,d_y_ant,d_z_ant] = calcDataFromText(parts,i,x_ant,y_ant,...
%         z_ant,d_x_ant,d_y_ant,d_z_ant);
%     end
%     
%     if ischar(dabs) && ischar(ddabs),
%         text = sprintf('\tNaN\tNaN\tNaN\tNaN\tNaN\tNaN\tNaN\tNaN');
%     elseif ischar(ddabs),
%         text = sprintf('\t%.2f\t%.2f\t%.2f\t%.2f\tNaN\tNaN\tNaN\tNaN',...
%             d_x,d_y,d_z,dabs);
%     else
%         text = sprintf('\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f',...
%             d_x,d_y,d_z,dabs,dd_x,dd_y,dd_z,ddabs);
%     end
%     
%     text = strcat(reports_saved{i},text);
%     fileText = sprintf('%s%s\n',fileText,text);
% end
% fprintf(f_id,'%s',fileText);
% fclose(f_id);



%% Replace adding the angles given two final feature files.

f_id=fopen(path3,'r');
reports=textscan(f_id, '%s', 'Delimiter', '\n');
fclose(f_id);
f=reports{1};

alphas = [-1 -1; 60 90; -1 -1];       % alpha1; alpha0; alpha2

f = calcLooksByAngles(f,alphas);

fid = fopen(finalPath, 'w');
for i = 1:length(f),
    fprintf(fid,sprintf('%s\n',f{i}));
end
fclose(fid);
