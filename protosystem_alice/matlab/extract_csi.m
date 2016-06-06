%
%simon <nicksimon109445@gmail.com>
%2015-5-21 16:51
%
%extract csi amplitude from samples 
function ret=extract_csi(csi)
    csi_scale=get_scaled_csi(csi);
    csi_scale=squeeze(csi_scale);
    for i=1:30
        ret(i)=abs(csi_scale(i));
    end
end