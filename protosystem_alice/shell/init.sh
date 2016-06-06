#!/bin/bash
#if [ $# -gt 2 ]; then
	#echo "Usage: ./init.sh num_packets delay_us"
#elif [ $# -lt 2 -o !NUM_PACKETS = $2 ]; then
#	NUM_PACKETS=100
#fi
#echo $NUM_PACKETS
#exit
#NUM_PACKETS
#DELAY_US
sudo pwd
base_dir=".."
cd $base_dir
/usr/local/MATLAB/R2011b/bin/matlab -nojvm -nodisplay -nosplash -r "run('matlab/main.m');exit"
exit
sudo bin/log_to_file sample/alice.dat &
CAP_PID=$!
sudo bin/random_packets 100
sudo kill -2 $CAP_PID

echo "wait client for CSI file"
sudo bin/rv sample/bob.dat

echo "call matlab to generate keys"
#system may not add the environments, so specify the full path
/usr/local/MATLAB/R2011b/bin/matlab -nojvm -nodisplay -nosplash -r "run('matlab/main.m');exit"

echo "transmit key"
sudo bin/tx key/bob
