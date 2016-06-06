%
%simon <nicksimon109445@gmail.com>
%2015-05-15 23:32
%
function ret=seq_final(sam,list,q)
    global m;
    ret(1)=0;
    k=1;
    for i=1:size(list,2)
        pos=list(i);
        rss=sam(pos);
        flag=quant(rss,q);
        if(flag==0)
            i=i+1;
            continue;
        end
        j=1;
        while(j<m)
            rss=sam(pos+j);
            if(flag~=quant(rss,q))
                break;
            end
            j=j+1;
        end
        if(j==m)
            ret(k)=pos;
            k=k+1;
        end
    end
end