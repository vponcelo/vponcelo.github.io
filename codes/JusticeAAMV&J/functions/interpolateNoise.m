function lines = interpolateNoise(lines,c,i,b,part)

%% output:
% lines: lines of the file
%% input
% lines: lines of the file
% c: counter of lines to be interpolated
% i: position of the current line
% a: old value in the position i - c
% part: index of the single string of a line

%% 
if i - c > 1,   % is not the begining of the file
    parts = regexp(lines{i-c-1},'\t','split');
    a = str2double(parts{part});
    displacement = b - a;
    div_interpol = displacement / c;
    for j = 1:c,
        if i == length(lines),
            interp_value = a;
        else
            interp_value = a + j * div_interpol;
        end
        parts = regexp(lines{i-c-1+j},'\t','split');
        parts{6} = '3';
        if part == 5,
            parts{part} = sprintf('%d',round(interp_value));
        else
            parts{part} = sprintf('%.2f',interp_value);
        end
        text = parts{1};
        for k = 2:length(parts), 
            text = sprintf('%s\t%s',text,parts{k});
        end
        lines{i-c-1+j} = text;       
    end
    if i == length(lines),
        parts = regexp(lines{i},'\t','split');
        parts{6} = '3';
        if part == 5,
            parts{part} = sprintf('%d',round(interp_value));
        else
            parts{part} = sprintf('%.2f',interp_value);
        end
        text = parts{1};
        for k = 2:length(parts), 
            text = sprintf('%s\t%s',text,parts{k});
        end
        lines{i} = text;  
    end
else
    parts = regexp(lines{i},'\t','split');
    interp_value = parts{part};
    for j = 1:c,    % i has to be 1 (begining of the file)
        parts = regexp(lines{j},'\t','split');
        parts{part} = interp_value; parts{6} = '3';
        text = parts{1};
        for k = 2:length(parts), 
            text = sprintf('%s\t%s',text,parts{k});
        end
        lines{j} = text;   
    end
end