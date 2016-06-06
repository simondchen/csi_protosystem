%
% simon <nicksimon109445@gmail.com>
%2015-05-15 21:17
%
%quantize the samples
%s--->samples , q--->qp,qs
function ret=seq(sam,q)
    global m;
    ret(1)=0;
	k=1;
    j=0;
    flag=0;
    rss=0;
    len = size(sam,2);
    i=1;
    %for i=1:len-m+1
    while(i<=len-m+1)
        rss=sam(i);
        flag=quant(rss,q);
        if(flag==0)
            i=i+1;
            continue;
        end
        j=1;
        while(j<m)
            rss=sam(i+j);
            if(flag~=quant(rss,q))
                break;
            end
            j=j+1;
        end
		if(j==m)
			ret(k)=i;
			k=k+1;
			i=i+m;
        else
            i=i+1;
		end
    end
end
