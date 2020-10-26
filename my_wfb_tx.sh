#!/bin/bash

NICS_LIST=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb | nice grep -v intwifi | nice grep -v wlan | nice grep -v relay | nice grep -v wifihotspot`

echo "Start MY TX $NICS_LIST \n"
/usr/local/share/cameracontrol/IPCamera/svpcom_wifibroadcast/wfb_tx -u 6002 -p 60 -K /tmp/tx.key $NICS_LIST
	