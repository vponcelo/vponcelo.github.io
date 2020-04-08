

addpath('SubFunctions');
addpath('HaarCascades');

%% Compile, mex linking and load AAM model
% compile.m should work for Linux and Mac.
% To Windows users:
% If you are using a Windows machine, please use the basic convolution (fconv.cc).
% This can be done by commen+ting out line 13 and uncommenting line 15 in
% compile.m
% compile;

% load and visualize model
% Pre-trained model with 146 parts. Works best for faces larger than 80*80
load face_p146_small.mat

% % Pre-trained model with 99 parts. Works best for faces larger than 150*150
% load face_p99.mat

% % Pre-trained model with 1050 parts. Give best performance on localization, but very slow
% load multipie_independent.mat

%% Parameters Initialization
% 5 levels for each octave
model.interval = 5;
% set up the threshold
% model.thresh = min(-0.65, model.thresh);
model.thresh = min(-0.95, model.thresh);

% constants of filtering parameters
f_params.OFFSET_PIXELS_AAM = 50;
f_params.OFFSET_DEGREE_POSE = 120;
f_params.OFFSET_INTERSECT_UNION = 0.2;
f_params.PERSONS = 1;
f_params.POST = false;
f_params.MIN_HITS_NEEDED = 3;
f_params.MAX_ERRORS_ALLOWED = 3;       
f_params.VIDEO_SECONDS = 100;             % Video Seconds
f_params.FPS = 10;                        % = Frame Rate
f_params.MAX_LOST_FRAMES_ALLOWED = f_params.FPS*f_params.VIDEO_SECONDS; 

% control vars of filtering parameters
f_params.hits = zeros(1,f_params.PERSONS);
f_params.falsePositives = zeros(1,f_params.PERSONS);
f_params.falseNegatives = zeros(1,f_params.PERSONS);


% Viola & Jones cascade models paths
FilenameHaarcascade = {'HaarCascades/haarcascade_frontalface_alt.mat',...
    'HaarCascades/haarcascade_frontalface_alt2.mat',...
    'HaarCascades/haarcascade_profileface.mat'};

dataPath = './outputPath/';
imgNum = 2000;
root = './MatImSeqPath/';
runFDVJAAM_mat(root,dataPath,imgNum,FilenameHaarcascade,model,f_params);

