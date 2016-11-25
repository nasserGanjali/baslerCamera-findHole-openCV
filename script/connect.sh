#!/bin/bash
#T1=0
#T2=0
sudo ifconfig wlan0 down >/dev/null 2>&1
if [ $? -eq 0 ] 
then
	sudo ip addr add 192.168.1.11/24 dev eth0 >/dev/null 2>&1
	if [ $? -eq 0 ] 
	then
		echo 0
	else 
		echo 1
	fi
else
	echo 1
fi
