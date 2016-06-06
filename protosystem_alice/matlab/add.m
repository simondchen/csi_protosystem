function ret=add(vec)
    ret(1) = 0;
    for i=1:size(vec,2)
        ret = vec(i);
    end
end