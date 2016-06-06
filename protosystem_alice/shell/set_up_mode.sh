#!/usr/bin/sudo /bin/bash
#sudo rmmod iwlwifi mac80211 cfg80211
#sudo modprobe -r iwlwifi mac80211 cfg80211
#sudo modprobe -r iwlwifi
#sudo modprobe iwlwifi debug=0x40000 connector_log=0x1
#sudo modprobe iwlwifi connector_log=0x1
#wait for the iwlwifi loaded
#sleep 1
# Setup monitor mode, loop until it works
sudo rfkill unblock wifi
sudo service network-manager stop
sudo ifconfig wlan0 down
sudo iwconfig wlan0 mode monitor 2>/dev/null 1>/dev/null
while [ $? -ne 0 ]
do
	sudo iwconfig wlan0 mode monitor 2>/dev/null 1>/dev/null
done
sudo ifconfig wlan0 up
sudo iw wlan0 set channel 64 HT20
#sudo echo 0x4101 | tee /sys/kernel/debug/ieee80211/phy1/iwlwifi/iwldvm/debug/monitor_tx_rate
sudo echo 0x4101 | tee `find /sys -name monitor_tx_rate`
#sudo echo 0x01 | tee /sys/kernel/debug/ieee80211/phy1/iwlwifi/iwldvm/debug/rx_chains_msk
sudo echo 0x01 | tee `find /sys -name rx_chains_msk`
