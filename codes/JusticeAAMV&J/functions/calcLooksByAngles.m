function f = calcLooksByAngles(f,alphas)
% Calculate probability of looking each possible person given thresholds
%% output
% f: file array with features of current part.
%% input:
% f: text array with features of current part.
% alphas: vector with the final flag of current part indicating the look 

%%

n_lines = length(f); 

for i = 1:n_lines,
    parts = regexp(f{i},'\t','split');
    angle = str2double(parts{5});
    for j = 1:size(alphas,1),
        if alphas(j,1) == -1 && alphas(j,2) == -1,
            f{i} = strcat(f{i},sprintf('\t0'));                 
        elseif angle >= alphas(j,1) && angle <= alphas(j,2),
            f{i} = strcat(f{i},sprintf('\t1'));
        else
            f{i} = strcat(f{i},sprintf('\t0'));
        end
    end
end
