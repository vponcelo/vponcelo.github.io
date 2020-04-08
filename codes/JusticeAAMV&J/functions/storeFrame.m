function fileID = storeFrame(fileID,aam,i,manual,D,posemap,confidence)
% Dump AAM features for all frames in a file

%% Input
% aam features of the current frame i to a given opened fileID
% manual: x,y coordinates manually anotated by the user
% D: Depth frame
% posemap: mapping from view-specific mixture id to viewpoint
% confidence: Confidence of the detection
%% Output
% Opened file

line = int2str(i);

if ~isempty(manual) && ~isempty(D),
%     depthValue = D(round(manual.y),round(manual.x));
    depthValue = getMedianDepth(manual.x,manual.y,D,10);
    v = depthToWorld(manual.x,manual.y,depthValue);
    status = 2; confidence = 1.00;
    if ~isempty(aam),
        angle = posemap(aam.c);
    else
        angle = -1;
    end        
elseif ~isempty(aam) && ~isempty(D),
    x = (aam.xy(:,1)+aam.xy(:,3))/2;
    y = (aam.xy(:,2)+aam.xy(:,4))/2;
    x_min = min(x); x_max = max(x); y_min = min(y); y_max = max(y);
    x_cm = (x_min+x_max)/2; y_cm = (y_min+y_max)/2;
    plot(x_cm,y_cm,'.','MarkerSize',30,'Color','g');
%     depthValue = D(round(y_cm),round(x_cm));
    depthValue = getMedianDepth(round(x_cm),round(y_cm),D,10);
    v = depthToWorld(x_cm,y_cm,depthValue);
    angle = posemap(aam.c);
    status = 1;
else
    v.x = -1; v.y = -1; v.z = -1; angle = -1; status = 0;  
end

line = strcat(line,sprintf('\t%.2f\t%.2f\t%.2f\t%d\t%d\t%.2f',v.x,v.y,v.z,angle,status,confidence));
line = strcat(line,'\n');

fprintf(fileID,line);
