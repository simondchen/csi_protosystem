function ret=total_rss(csi_st)
    rssi_mag = 0;
    if csi_st.rssi_a ~= 0
        rssi_mag = rssi_mag + dbinv(csi_st.rssi_a);
    end
    if csi_st.rssi_b ~= 0
        rssi_mag = rssi_mag + dbinv(csi_st.rssi_b);
    end
    if csi_st.rssi_c ~= 0
        rssi_mag = rssi_mag + dbinv(csi_st.rssi_c);
    end
    ret = db(rssi_mag, 'pow');
end