function depthValue = getMedianDepth(x,y,D,n)
% Look for the median value of depth for a given grid of size n x n

%% output
% depthValue: median of a sorted list of depth values
%% input
% x: original x coordinate of the pixel value
% y: original y coordinate of the pixel value
% D: depth Image
% n: size of the grid

x_vec = (x - n/2):1:(x + n/2);
y_vec = (y - n/2):1:(y + n/2);
depthValues = zeros(1,length(x_vec));
for i = 1:length(depthValues),
    depthValues(i) = D(round(y_vec(i)),round(x_vec(i)));
end
depthValues = sort(depthValues);
depthValue = median(depthValues);