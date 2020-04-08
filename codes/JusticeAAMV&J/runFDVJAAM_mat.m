function runFDVJAAM_mat(root,dataPath,imgNum,FilenameHaarcascade,model,filterParams)
% Face detect Viola & Jones and AAM Ramanan for mat images

% Input: 
    % root:         path data directory
    % dataPath:     output data path directory
    % imgNum:       initial image number for testing
    % FilenameHaarcascade: mat file with the constructed cascade of
        % classifiers
    % model:        AAM pre-trained model from MultiPIE dataset

    
% disp('Model visualization');
% visualizemodel(model,1:13);
% disp('press any key to continue');
% pause;

% define the mapping from view-specific mixture id to viewpoint
if length(model.components)==13 
    posemap = 90:-15:-90;
elseif length(model.components)==18
    posemap = [90:-15:15 0 0 0 0 0 0 -15:-15:-90];
else
    error('Can not recognize this model');
end

b_ant = [];
b_ant2 = [];
fprintf('loading %s ...\n',root'); 
ims = dir(sprintf('%s/*.mat',root));

output = sprintf('%s/faces_mat_%.2f',dataPath,model.thresh);
if ~exist(output,'dir'),        
    mkdir(output);
end

%% Creating Output File for storing features
id_file1 = fopen(sprintf('%s/p1.txt',dataPath),'a');
if filterParams.PERSONS > 1,
    id_file2 = fopen(sprintf('%s/p2.txt',dataPath),'a');
end

%% Creating Video file for the k:k+1000 frames
% myMovie = cell(1,1000); k = 14770;
% myVideo = VideoWriter(sprintf('%s/faces_mat_%.2f.avi',dataPath,model.thresh));
% myVideo.FrameRate = filterParams.FPS;
% open(myVideo);

for i = imgNum:length(ims)-1,    
    %% Obtaining index to handle with sorted filenames
    
    idx = ims(i).name(end-15:end) == '_';
    j = 4;
    while ~idx(end-j),
        j = j + 1;
    end    
    
    outputFile = sprintf('%s/%d.jpg',output,i);

    %% Obtaining image directly from mat files
    if ~exist(outputFile,'file'),
        fprintf('\ntesting image %d/%d ...\n', i, length(ims));
        
        load(sprintf('%s/%s%d.mat',root,ims(i).name(1:end-j),i));
        I = ColorFrame.ColorData;      

        Objects = cell(1,3);
%         tic;
%         for j = 1:length(FilenameHaarcascade),            
%             Objects{j}=ObjectDetection(I,FilenameHaarcascade{j});            
%         end 
%         dettime = toc;                        
%         fprintf('Viola and Jones detections took %.1f seconds\n\n',dettime);
        
        if ~isempty(b_ant) || ~isempty(b_ant2) || i >= imgNum,
            tic;
            bs = detect(I, model, model.thresh);
            bs = clipboxes(I, bs);
            bs = nms_face(bs,0.3);
            dettime = toc;
            fprintf('AAM Ramanan detections took %.1f seconds\n\n',dettime);
        else
            bs = [];
        end
        imagesc(I);
        showboxes(bs,posemap,Objects);
        
        %% heuristic function that selects the best faces and discards bad faces 
        display('Person 1');
        
        [b,b_ant,winner,filterParams,manual,confidence] = filterFaces(bs,b_ant,filterParams,posemap,1);
        
        if ~isempty(manual) || ~isempty(b), D = DepthFrame.DepthData; else D = []; end;
         
        close all; imagesc(I);        
        showboxes(b,posemap,Objects,manual);
        storeFrame(id_file1,b,i,manual,D,posemap,confidence);
        
        if filterParams.PERSONS > 1,
            if winner > 0,
                bs(winner) = [];            
            end
            display('Person 2');
            [b,b_ant2,winner,filterParams,manual,confidence] = filterFaces(bs,b_ant2,filterParams,posemap,2);
            if ~isempty(manual) || ~isempty(b), D = DepthFrame.DepthData; else D = []; end;
            showboxes(b,posemap,Objects,manual);            
            storeFrame(id_file2,b,i,manual,D,posemap,confidence);                    
        end
        
        %% Saving the output images in the specified directory
        saveas(gcf,outputFile);
        
        %% Create a 1000 frames movie
%         if k <= length(myMovie),
%             myMovie{k} = getframe(gcf);  
%             if k == length(myMovie),
%                 writeVideo(myVideo, cell2mat(myMovie));     
%                 close(myVideo);
%             end
%             k = k + 1;
%         end         
        
        %% User control
%         disp('press any key to continue');
%         pause;        
        close all;   
    else
        imgNum = i + 1; b_ant = []; b_ant2 = [];
        filterParams.hits = zeros(1,filterParams.PERSONS);
        filterParams.falsePositives = zeros(1,filterParams.PERSONS);
        filterParams.falseNegatives = zeros(1,filterParams.PERSONS);
    end    
end
fclose(id_file1);
% fclose(id_file2);