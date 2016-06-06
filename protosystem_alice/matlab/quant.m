%
%simon <nicksimon109445@gmail.com>
%2015-05-15 22:27
%
function ret=quant(rss,q)
    qp=q(1);
    qs=q(2);
    if(rss>qp)
        ret=1;
    elseif(rss<qs)
        ret=-1;
    else
        ret=0;
    end
end
