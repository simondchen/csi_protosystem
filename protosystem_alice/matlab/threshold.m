%
% simon <nicksimon109445@gmail.com>
%2015-05-15 21:12
%
%compute q+ and q-
function ret=threshold(vec)
    alpha=0.5;
    m=mean(vec);
    s=std(vec);
    ret(1)=m+alpha*s;
    ret(2)=m-alpha*s;
end