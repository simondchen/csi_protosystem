%
%2015年 10月 20日 星期二 13:54:49 CST
%<nicksimon109445@gmail.com>
%draw the rss samples
% 
%function draw_seq(fname1,fname2)
function draw_seq(data1,data2)
    %data=load('/home/simon/seq_initiator')
    %data=data'
     %x=1:size(data,2)
    x=1:137
    %x=130:200
    set(gca,'XTick',0:2:30)
    y1=data1
    plot(x,y1,'-go')
   %data=load('/home/simon/seq_responser')
    %data=data'
    y2=data2
    %qa=threshold(y1)
    %qb=threshold(y2)  
    hold on
    plot(x,y2,'-ro')
    %line1=qa(1)*ones(1,length(x))
    %plot(x,line1,'-g')
    %line2=qa(2)*ones(1,length(x))
    %plot(x,line2,'-g')
    %line3=qb(1)*ones(1,length(x))
    %line4=qb(2)*ones(1,length(x))
    %plot(x,line3,'-r')
    %plot(x,line4,'-r')
end
