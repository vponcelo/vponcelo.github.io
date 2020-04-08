function showboxes(boxes,posemap,Objects,manual)

% showboxes(boxes)
% Draw boxes on top of image.

hold on;
axis image;
axis off;

% Show the detected objects from Ramanan AAM 
for b = boxes,
    partsize = b.xy(1,3)-b.xy(1,1)+1;
    tx = (min(b.xy(:,1)) + max(b.xy(:,3)))/2;
    ty = min(b.xy(:,2)) - partsize/2;
    text(tx,ty, num2str(posemap(b.c)),'fontsize',18,'color','c');
    for i = size(b.xy,1):-1:1;
        x1 = b.xy(i,1);
        y1 = b.xy(i,2);
        x2 = b.xy(i,3);
        y2 = b.xy(i,4);
        line([x1 x1 x2 x2 x1]', [y1 y2 y2 y1 y1]', 'color', 'b', 'linewidth', 1);        
        plot((x1+x2)/2,(y1+y2)/2,'r.','markersize',15);
    end
end

% Show the detected objects from Viola & Jones frontal, frontal2 and profile
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

if exist('manual','var'),
    if ~isempty(manual),
        plot(manual.x,manual.y,'.','MarkerSize',30,'Color','g');
    end
end

drawnow;
