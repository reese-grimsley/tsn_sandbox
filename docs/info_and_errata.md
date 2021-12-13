# Info and Errata

This page is dedicated to a lower priority information and known errors.

## Additional Info

### Switch

* The TSN switch will store its current configuration in memory, and upon reset, load the configuration held in NVM (Non-Voaltile Memory). This means that if the configuration is not saved to NVM, it will need to be reconfigured when it reboots. By default, the switch does not synchronize these the configurations, but will if the appropriate box is checked in the configuration management interface (the save/floppy-disk icon in the upper left).

### VLAN

* VLAN priorities and Linux traffic classes are not the same. The network interface will translate between these using an egress and ingress mapping. By default, all but 0, 1, and 2 are mapped one-to-one (e.g., traffic class 5 -> priority 5; assume traffic class > 7 maps to priority 7). The popular convention is to consider traffic class 0 (default in Linux) as priority 2 (best effort), class 1 as priority 0, and class 2 as priority 1. Strange, but consistent. The TSN switch we use follows the same convention. In VLAN configuration terms, that would look like a QoS mapping of 0:2 1:0 2:1 3:3 4:4 ... .

## Errata

* When the nodes are configured to use 802.1AS (*i.e.*, gPTP) or PTP, and time sync is turned off through the switch's interface, it crashes. This typically only happens when endpoints are sending/expecting PTP messages to/from the switch.
  * The switch does not appear to recover on its own, not can it be accessed through any of its network ports. The best solution thus far has been to power cycle (unplug, wait a few seconds, plug back in).
* The TSN switch fails to act as a proper grandmaster in PTP or gPTP. It is recommended to give one or more of the endpoints a higher priority (lower value) than the switch.  
  * When the switch is acting as an endpoint, the endpoints (running ptp4l for PTP) show an issue with calibration in the master and/or "ANNOUNCE_RECEIPT_TIMEOUT_EXPIRES". At that point, the endpoints will select themselves as grand master and the network will not be synchronized
  * Additionally, the TSN switch is not a great choice for grandmaster as it may or may not have a connection to global time via UTC.
* The [phc2sys documentation](http://manpages.ubuntu.com/manpages/bionic/man8/phc2sys.8.html) states that phc2sys can wait (-w) until ptp is synchronized before it performs local synchronization. However, this has not been found to work; it never finds ptp to be synchronized, regardless of what ptp4l logs show. 
  * The -w tag for phc2sys is supposed to determine the correct offset between UTC and NIC clocks on its own, but we do not observe this. As such, the offset is manually configured (positive in the master, negative in the clients). Omitting this results in clock synchronization errors on the order of 2x #Leap-Seconds.