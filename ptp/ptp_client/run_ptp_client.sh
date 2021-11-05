#!/bin/bash

if [[ `whoami` != "root" ]]; then
    echo "Run as root/sudo!"; exit -1;
fi


sudo ptp4l -f ptp_client.conf -i enp87s0 -m &> ptp.log &
echo "ptp4l is running in bg as pid $!"

sudo phc2sys -s enp87s0 -c CLOCK_REALTIME -w -m &> phc2sys.log & 
echo "phc2sys (sync NIC clock to sys clock) is running in bg as pid $!"