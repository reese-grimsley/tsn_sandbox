#!/bin/bash

if [[ `whoami` != "root" ]]; then
    echo "Run as root/sudo!"; exit -1;
fi

echo "Start ptp..."
sudo ptp4l -f ptp_master.conf -i enp87s0 -m &> ptp.log &
echo "ptp4l is running in bg as pid $!"

sleep 2

echo "Start phc2sys..."
sudo phc2sys -c enp87s0 -s CLOCK_REALTIME -w -m &> phc2sys.log & 
echo "phc2sys (sync NIC clock to sys clock) is running in bg as pid $!"
