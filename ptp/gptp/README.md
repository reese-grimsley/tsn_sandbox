# Clock Synchronization via GPTP aka 802.1AS

As described in the [parent README](../README.md), we use ```ptp4l``` and ```phc2sys``` commands in Linux to synchronize the clocks in the network interface between devices and the system clock to NIC clock per device, respectively.

The system to NIC clock configuration (using phc2sys) depends on whether the device is acting as the Grand Master or not. The TSN switch itself have [shown issues](../../info_and_errata.md#errata) with being the Grand Master, so it is best to designate one of the endpoint devices as the Grand Master instead.

To run clock synchronization in a linux endpoint, execute one of the SH scripts in this directory with root permissions.

## Contents

```run_gptp_client.sh ``` will run the two synchronization commands under the assumption that the device *is not* the Grand Master clock.
```run_gptp_master.sh ``` will run the two synchronization commands under the assumption that the device *is* the Grand Master clock.

The two have their own respective configuration (```gPTP_client.cfg``` and ```gPTP_master.cfg```) files for ptp4l, but the only practical difference is the priority the device takes in the Best Clock Master algorithm within PTP.

Both of these shell scripts must be run with root permissions.

## PTP4L

The ptp4l configuration is exactly the same as the recommended one for gPTP, save that the one deisngated as master has a higher priority for the Best Master Clock Algorithm.

Once started, ptp4l will run in the background, redirecting its output into a log file, ```ptp.log```. It may take a moment to synchronize. It is helpful to follow the log with tail to debug: ```tail -f -n 25 ptp.log```. The log will show statistics about the quality of synchronization, the delay, frequency offset, etc. for client devices, but will be mostly bare for the master.

## PHC2SYS

Like ptp4l, this will run as a background process once started, redirecting its output to phc.log.

In the master, the NIC clock is synchronized to the system clock, which we expect to be synchronized to UTC via ```ntpd``` or ```chrony``` (out of scope for this document). This includes a positive offset to account for Leap Seconds.

In the client, the system clock is synchronized to the NIC clock. There should not be any other clock synchronization service running in the background.

We mentioned in the [errata](../../info_and_errata.md#errata) (based on experience) that several of the default options do not work, hence more manual configuration within the scripts

## TSN Switch Configuration

Note that these instructions are specific to **BRS40-8TX** managed L2 switch running their custom OS, HiOS (version 8.7.02-S).

The switch has a set of configurations for "Time" (1) as shown in the image below (TODO), one of which is 802.1AS (gPTP).

<TODO> image

To enable gPTP, simply click the button that says to enable (2) and then the save icon along the bottom (3). To monitor the quality of synchronization you can reload the page with an icon at the bottom (4), and parameters within this screen will update to show values like the offset to master, the relative oscillator frequency, and whether the device considers itself to be **synchronized** (5) based on a set of upper and lower hysteresis thresholds.

Note that the switch must consider itself to be synchronized to use any of the TSN functions under Switching -> TSN, such as TDMA-style QoS.

## Debugging

Occasionally, clock synchronization will degrade substantially. This typically only happens after several days or weeks of continuous synchronization. It stands to reason that the switch is responsible, as [restarting via power-cycling](../info_and_errata.md#errata) it often fixes the issue. Stopping the phc2sys and ptp4l services on the endpoints is recommended when this happens (```sudo killall ptp4l phc2sys```). Once the switch is back on and gPTP enabled, restart the shell scripts.
