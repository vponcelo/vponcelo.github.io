function [D]=toMat(seqs)
    D=[];
    for s=seqs,
        D=[D;cell2mat(s)];
    end