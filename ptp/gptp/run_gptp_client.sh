#!/bin/bash

echo "Run gPTP and ph2csys, assuming client role"

if [[ `whoami` != "root" ]]; then
    echo "Run as root/sudo!"; exit -1;
fi

echo "Start ptp..."
sudo ptp4l -f gPTP_client.cfg -i enp87s0 -m &> ptp.log &
echo "ptp4l is running in bg as pid $!"

sleep 2

echo "Start phc2sys..."
sudo phc2sys -s enp87s0 -c CLOCK_REALTIME -w -m -O -37 &> phc2sys.log &  # assume as client configuration... but what if this device is grand-master?
echo "phc2sys (sync NIC clock to sys clock) is running in bg as pid $!"