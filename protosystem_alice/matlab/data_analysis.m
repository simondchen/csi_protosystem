%
%simon <nicksimon109445@gmail.com>
%2015-05-22 12:17
%
%analyze the property of generated keys
function data_analysis(key)
    num=0;
    alice=key(1,:);
    bob=key(2,:);
    for i=1:size(alice,2)
        if(alice(i)~=bob(i))
            num=num+1;
        end
    end
    percentage=(1-num/size(alice,2))*100;
    fprintf('%f\n',percentage);
end