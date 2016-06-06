%
%simon <nicksimon109445@gmail.com>
%2015-5-21 17:45
%
%extract specfic subcarrier csi sample
function ret=extract_m_csi(entry,index)
    for i=1:size(entry,2)
        csi=entry(i);
        csi=get_scaled_csi(csi);
        csi=squeeze(csi);
        csi=abs(csi);
        ret(i)=csi(index);
    end
end