#!/bin/bash

NICS_LIST=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb | nice grep -v intwifi | nice grep -v wlan | nice grep -v relay | nice grep -v wifihotspot`

echo "Start MY RX $NICS_LIST \n"
/usr/local/share/cameracontrol/IPCamera/svpcom_wifibroadcast/wfb_rx -k 8 -n 4 -c 127.0.0.1 -u 6001 -p 60 -K /tmp/rx.key $NICS_LIST
	