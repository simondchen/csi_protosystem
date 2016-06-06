function draw(csi_alice,csi_bob)
     csi_scale=get_scaled_csi(csi_alice);
     plot(db(abs(squeeze(csi_scale).')));
     hold on;
     csi_scale=get_scaled_csi(csi_bob);
     plot(db(abs(squeeze(csi_scale).')));
end