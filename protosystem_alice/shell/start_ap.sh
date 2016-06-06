sudo ifconfig wlan1 up
sudo ifconfig wlan1 10.10.0.1 netmask 255.255.255.0
while [ $? -ne 0 ]
do
	sudo ifconfig wlan1 10.10.0.1 netmask 255.255.255.0
done
sudo service isc-dhcp-server start
sudo hostapd /etc/hostapd/hostapd.conf
