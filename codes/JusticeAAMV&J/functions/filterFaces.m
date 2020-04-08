function [b,b_ant,i,params,manual,confidence] = filterFaces(bs,b_ant,params,posemap,person)
%% input:
%   bs: AAM struct with the current detections
%   b_ant: AAM struct with the previous detection
%   params: parameters and vars of the current session
%   posemap: set of possible poses
%   person: current person of the frame (1 or 2)
%% output
%   b: output winner AAM current struct
%   b_ant: output AAM previous struct
%   i: Index of the winner AAM b
%   params: updated vars of the current session

manual = [];

% Avoid false positives (noise removal)
if ~isempty(bs),    
    i = 0; 
    offset_aam = zeros(1,length(bs));
    inters_aam = zeros(1,length(bs)); inters_aam(:) = -1;
    
    % Loop over all AAM faces selecting the closest to the previous one by
    % calculating the offset distance between both AAMs and poses.
    for b = bs;    
        i = i + 1;
        % Calculation of Center of Masses for current and previous AAMs
        x = (sum(b.xy(:,1)+b.xy(:,3))/size(b.xy,1))/2;      
        y = (sum(b.xy(:,2)+b.xy(:,4))/size(b.xy,1))/2;      
        
        % Manually select the Center of Mass of the AAM for the first image
        if isempty(b_ant),  
            params.hits(person) = 0;
            if ~exist('x_ant','var'),
                [x_ant,y_ant] = ginput(1);   % Manually CM selected by the user               
                manual.x = x_ant; manual.y = y_ant;
            end            
            
            % Asssign b_ant if it's close to the user's CM. Otherwise, 
            % higher offset is expected to be assigned and it will continue
            % the search for the next detections.
            if sqrt((x-x_ant)^2 + (y-y_ant)^2) < params.OFFSET_PIXELS_AAM/3,
                b_ant = b;                
            end
        else
            x_ant = (sum(b_ant.xy(:,1)+b_ant.xy(:,3))/size(b_ant.xy,1))/2;
            y_ant = (sum(b_ant.xy(:,2)+b_ant.xy(:,4))/size(b_ant.xy,1))/2;
            [inters_aam(i),h1,h2] = calcAreas(b,b_ant);
            delete(h1); delete(h2); 
        end
        offset_aam(i) = sqrt((x-x_ant)^2 + (y-y_ant)^2);
    end
    
    if ~isempty(b_ant),
        [offset_aam,i] = min(round(offset_aam));
        if inters_aam(i) == -1, inters_aam(i) = 1; end;
        b = bs(i);
        offset_pose = abs(posemap(b.c)-posemap(b_ant.c));
        fprintf('Offset: %d\t Intersection/Union: %.2f\n',offset_aam,inters_aam(i));
        % Correct Detection below the threshold values
        if offset_aam <= params.OFFSET_PIXELS_AAM && ...
                offset_pose <= params.OFFSET_DEGREE_POSE && ...
                inters_aam(i) >= params.OFFSET_INTERSECT_UNION,
            if offset_aam < params.OFFSET_PIXELS_AAM/6 || params.POST, 
                params.hits(person) = params.MIN_HITS_NEEDED;
                display('Best AAM detected !!');
            else
                params.hits(person) = params.hits(person) + 1;
            end
            params.falsePositives(person) = 0; 
            params.falseNegatives(person) = 0;  
        else        % False Positive case
            i = -1;
            params.falsePositives(person) = params.falsePositives(person) + 1;            
            display('False Positive');              
            b = b_ant;                  
        end
    else
        i = -1; params.hits(person) = 0;
    end
    
% Avoid False Negatives displaying the previous detection if it exists and
% the number of errors is below the threshold
else
    i = -1;
    params.falseNegatives(person) = params.falseNegatives(person) + 1;    
    display('False Negative');
    if ~isempty(b_ant),
        b = b_ant;            
    else
        params.hits(person) = 0;
        [x_ant,y_ant] = ginput(1);   % Manually CM selected by the user               
        manual.x = x_ant; manual.y = y_ant;        
    end
end

% Avoid either not validated faces or false negatives
errors = params.falseNegatives(person) + params.falsePositives(person);
p_errors = errors/2/params.MAX_ERRORS_ALLOWED;
if params.hits(person) < params.MIN_HITS_NEEDED, 
    confidence = params.hits(person)/params.MIN_HITS_NEEDED;
    if confidence - p_errors >= 0,
        confidence = confidence - p_errors;
    end
else        
    confidence = 1 - p_errors;   
end
if confidence == 0, params.hits(person) = 0; end

fprintf('Confidence: %.2f\n',confidence);
if confidence <= 0.5,
    b = [];       % If it is commented, hits will be disabled
    display('Not Drawing AAM');     
else
    display('Drawing AAM !');
    b_ant = b;
end

% Request User Annotation on the next iteration with AAM detections due to
% excess of frames either with false negatives or false positives 
if errors >= params.MAX_LOST_FRAMES_ALLOWED,
    b_ant = [];
    params.hits(person) = 0;
    display('Maximum Lost Frames exceeded. User Anotation required now ...');
end
