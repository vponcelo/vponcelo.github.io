function winner = calcWinner(delIndexs,totalIdxs,i_opt)
% Calculate the best original index selected within all the detected boxes 

%% output
% winner: original index of the winner box
%% input
% delIndexs: array with the deleted boxes
% totalIdxs: length of the correct boxes
% i_opt: index of the optimal box after deleting

if ~isempty(delIndexs),
    j = 1; 
    found = false; 
    k = 1;
    while j <= length(delIndexs) && ~found,
        if i_opt >= delIndexs(j),
            k = k + 1;
        else 
            while k < delIndexs(j)-1,
                k = k + 1;
            end
            if k == totalIdxs,
                found = true;
            end
        end   
        j = j + 1;
    end
else
    k = i_opt;
end

winner = k;
