#!/bin/bash
### BEGIN INIT INFO
# Provides:          RoboMobileAS
# Required-Start:    $all
# Required-Stop:     $all
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: RoboMobile autostart script
# Description:       RoboMobile autostart script
### END INIT INFO

PATH_TO_PROG=/home/orangepi/RoboMobile/

rb_start ()
{
	#Initial wifi interface configuration
	i=1
	while [ -z "$(ifconfig -a | grep wlan0)" ]
	do
		if [ "$i" -gt 60 ]
		then
			break
		fi
		sleep 1
		let i=i+1
	done
	#echo 2 > /sys/module/bcmdhd/parameters/op_mode #only for firefly
	ifconfig wlan0 up 192.168.111.1 netmask 255.255.255.0
	sleep 2
	
	#start hostapd
	/usr/sbin/hostapd /etc/hostapd/hostapd.conf &
	
	#start dnsmasq
	if [ -z "$(ps -e | grep dnsmasq)" ]
	then
		/etc/init.d/dnsmasq start &
	fi
	
	#start RoboMobile	
	cd ${PATH_TO_PROG}
	cd build
	./RoboMobile
	if [ -z "$(ps -e | grep RoboMobile)" ]
	then
		echo "Success"
	else
		echo "Failed"
	fi
}

case "$1" in
start)
	echo "Starting RoboMobile..."
    rb_start
    ;;
stop)
	echo "Stopping RoboMobile..."
    killall RoboMobile
    ;;

restart|force-reload)
    echo "Restarting RoboMobile..."
    killall RoboMobile
    ;;
*)
     echo "Usage: `basename $0` {start|stop|restart|force-reload}" >&2    
     exit 1
    ;;   

esac

exit 0
