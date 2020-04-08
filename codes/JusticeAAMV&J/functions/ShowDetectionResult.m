function ShowDetectionResult(Picture,Objects)
%  ShowDetectionResult(Picture,Objects,color)
%
%

% Show the picture
figure,imshow(Picture), hold on;

% Show the detected objects
for i = 1:length(Objects),
    color = zeros(1,3);
    color(i) = 1;
    if(~isempty(Objects{i}));
        for n=1:size(Objects{i},1)
            x1=Objects{i}(n,1); y1=Objects{i}(n,2);
            x2=x1+Objects{i}(n,3); y2=y1+Objects{i}(n,4);
            plot([x1 x1 x2 x2 x1],[y1 y2 y2 y1 y1],'LineWidth',2,'Color',color);
        end
    end
end
