#!/bin/bash
#line after line
sudo pwd
base_dir=".."
cd $base_dir
sudo bin/log_to_file sample/bob.dat &
CAP_PID=$!
sudo bin/random_packets 100
echo "after random"
sudo kill -2 $CAP_PID

echo "wait for alice ready for recv CSI file"
sleep 1

echo "transmit CSI file"
sudo bin/tx samplebob.dat

echo "wait for key"
sudo bin/rv key/bob
