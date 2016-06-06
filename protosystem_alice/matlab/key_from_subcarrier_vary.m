%
%simon <nicksimon109445@gmail.com>
%2015-05-22 16:44
%
%generate key from samples vary with subcarrier
function ret=key_from_subcarrier_vary(alice,bob)
    ret=[0;0];
    for i=1:size(alice,2)
        csi_s_alice=extract_csi(alice(i));
        csi_s_bob=extract_csi(bob(i));
        generate_key(csi_s_alice,csi_s_bob);
        if(ans~=-1)
            ret=[ret ans];
        end
    end
end