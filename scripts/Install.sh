#!/bin/bash

#to run need root privilegies
echo "RoboMobile Intallation"

INST_DIR="/home/$(whoami)/"
YN=""

#go to install directory
cd ${INST_DIR}

#delete old installations
if [ -e "RoboMobile" ]
then
	rm -rf RoboMobile
fi

if [ -e "/etc/init.d/RoboMobileAS.sh" ]
then
	/etc/init.d/RoboMobileAS.sh stop
	update-rc.d -f RoboMobileAS.sh remove
	rm /etc/init.d/RoboMobileAS.sh
fi

#installing dependencies
apt-get install cmake libopencv-dev hostapd dnsmasq git

#stop dnsmasq and hostapd and remove it from autostart
service dnsmasq stop
service hostapd stop
update-rc.d -f dnsmasq remove
update-rc.d -f hostapd remove

#clone last RoboMobile from GitHub 
#git clone https://github.com/Keylost/RoboMobile

#configure connections
cp hostapd.conf /etc/hostapd/hostapd.conf
cp dnsmasq.conf /etc/dnsmasq.conf
cp interfaces /etc/network/interfaces

#adding robomobile to autostart
cp RoboMobileAS.sh /etc/init.d/RoboMobileAS.sh
chmod +x /etc/init.d/RoboMobileAS.sh
update-rc.d RoboMobileAS.sh defaults

apt-get remove network-manager

#go to work directory
cd RoboMobile

#building
mkdir build
cd build
cmake ..
make
echo "Intallation complete"

#start
echo "Do you want to start RoboMobile right now? [y/n]"
read YN
while [ "$YN" != "y" ] && [ "$YN" != "n" ]
do
	echo "Enter y or n: "
	read YN
done

if [ "$YN" = "y" ]
then
	/etc/init.d/RoboMobileAS.sh start
fi
