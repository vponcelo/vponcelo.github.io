function runFDVJAAM_jpg(root,dataPath,imgNum,FilenameHaarcascade,model)
% Face detect Viola & Jones and AAM Ramanan for jpg images

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


% 5 levels for each octave
model.interval = 5;
% set up the threshold
model.thresh = min(-0.65, model.thresh);
% model.thresh = -0.1;

% define the mapping from view-specific mixture id to viewpoint
if length(model.components)==13 
    posemap = 90:-15:-90;
elseif length(model.components)==18
    posemap = [90:-15:15 0 0 0 0 0 0 -15:-15:-90];
else
    error('Can not recognize this model');
end

% dataPath = 'C:/victor/thesis/datasets/Justicia/2012-07-17_Barcelona/r11_s1/denunciats2/rgb';
% dataPath = 'images';
% imgNum = 34000;

ims = dir(sprintf('%s/*.jpg',root));
for i = imgNum:imgNum+1000%length(ims)-1,    

    output = sprintf('%s/faces_jpg',dataPath);
    if ~exist(output,'dir'),        
        mkdir(output);
    end
    outputFile = sprintf('%s/%d.jpg',output,i);

    %% Obtaining image from jpg files
%     if ~exist(outputFile,'file'),
        fprintf('testing: %d/%d\n', i, length(ims));

        I = imread(sprintf('%s/%d.jpg',root,i));

        Objects = cell(1,3);
        tic;
        for j = 1:length(FilenameHaarcascade),            
            Objects{j}=ObjectDetection(I,FilenameHaarcascade{j});            
        end 
        dettime = toc;                        
        fprintf('Viola and Jones detection took %.1f seconds\n\n',dettime);
        
        tic;
        bs = detect(I, model, model.thresh);
        bs = clipboxes(I, bs);
        bs = nms_face(bs,0.3);
        dettime = toc;
        fprintf('AAM Ramanan detection took %.1f seconds\n\n',dettime);
        
        showboxes(I,bs,posemap,Objects);
%         saveas(gcf,outputFile);
        
        disp('press any key to continue');

        pause;
        close all;
%     end
end
