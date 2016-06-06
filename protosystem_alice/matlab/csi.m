% read files
csi_trace_alice=read_bf_file('sample/alice.dat')
csi_trace_bob=read_bf_file('sample/bob.dat')

%get same seq
for i=1:size(csi_trace_alice,1)
    csi_entry_alice = csi_trace_alice{i};
    seq_alice(i) = csi_entry_alice.seq;
end
for i=1:size(csi_trace_bob,1)
    csi_entry_bob = csi_trace_bob{i};
    seq_bob(i) = csi_entry_bob.seq;
end
j=1;
for i=1:max(max(seq_alice(:)),max(seq_bob(:)))
    if (find(seq_alice(:)==i) & find(seq_bob(:)==i))
        seq_both(j)=i;
        j=j+1;
    end
end

%filter csi
%alice
j=1;
for i=1:size(csi_trace_alice,1)
    csi_entry_alice = csi_trace_alice{i};
    seq = csi_entry_alice.seq;
    if(find(seq_both==seq))
        alice(j)=csi_entry_alice;
        j=j+1;
    end
end
%bob
j=1;
for i=1:size(csi_trace_bob,1)
    csi_entry_bob = csi_trace_bob{i};
    seq = csi_entry_bob.seq;
    if(find(seq_both==seq))
        bob(j)=csi_entry_bob;
        j=j+1;
    end
end
clear i j seq* csi_entry*;
csi_alice=extract_m_csi(alice,1);
csi_bob=extract_m_csi(bob,1);

%get rssi
for i=1:size(alice,2)
    entry = alice(1,i);
    alice_rssi(i)=entry.rssi_a;
end

for i=1:size(bob,2)
    entry = bob(1,i);
    bob_rssi(i)=entry.rssi_a;
end

%get rssi according perm
for i=1:size(alice,2)
    entry = alice(1,i);
    perm = entry.perm;
    big = perm(1);
    switch big
        case 1
	    alice_rss(i) = entry.rssi_a;
	case 2
	    alice_rss(i) = entry.rssi_b;
	case 3
	    alice_rss(i) = entry.rssi_c;
    end
end

%get rssi according perm
for i=1:size(bob,2)
    entry = bob(1,i);
    perm = entry.perm;
    big = perm(1);
    switch big
        case 1
	    bob_rss(i) = entry.rssi_a;
	case 2
	    bob_rss(i) = entry.rssi_b;
	case 3
	    bob_rss(i) = entry.rssi_c;
    end
end


%get all picture
j=1;
for i=1:3%size(seq_both,2)
    while (csi_trace_alice{j}.seq~=seq_both(i))
        j=j+1;
    end
    csi_entry_alice = csi_trace_alice{j};
    csi_alice = get_scaled_csi(csi_entry_alice);
    figure(i);
    hold on;
    plot(abs(squeeze(csi_alice(1,:,:)).'),'.');
    plot(abs(squeeze(csi_alice(1,:,:)).'));
end
j=1;
for i=1:3%size(seq_both,2)
    while (csi_trace_bob{j}.seq~=seq_both(i))
        j=j+1;
    end
    csi_entry_bob = csi_trace_bob{seq_both(i)};
    csi_alice = get_scaled_csi(csi_entry_bob);
    figure(i);
    hold on;
    plot(abs(squeeze(csi_alice(1,:,:)).'));
end


%量化算法

for i=1:size(csi_trace_alice)
    csi_entry(i)=csi_trace_alice{i};
end


