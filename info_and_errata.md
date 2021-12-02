# Info and Errata

This page is dedicated to a lower priority information and known errors.

## Additional Info

* The TSN switch will store its current configuration in memory, and upon reset, load the configuration held in NVM (Non-Voaltile Memory). This means that if the configuration is not saved to NVM, it will need to be reconfigured when it reboots. By default, the switch does not synchronize these the configurations, but will if the appropriate box is checked in the configuration management interface (the save/floppy-disk icon in the upper left)

## Errata

* When the nodes are configured to use 802.1AS (*i.e.*, gPTP) or PTP, and time sync is turned off through the switch's interface, it crashes. This typically only happens when endpoints are sending/expecting PTP messages to/from the switch.
  * The switch does not appear to recover on its own, not can it be accessed through any of its network ports. The best solution thus far has been to power cycle (unplug, wait a few seconds, plug back in).
* The TSN switch fails to act as a proper grandmaster in PTP or gPTP. It is recommended to give one or more of the endpoints a higher priority (lower value) than the switch.  
  * When the switch is acting as an endpoint, the endpoints (running ptp4l for PTP) show an issue with calibration in the master and/or "ANNOUNCE_RECEIPT_TIMEOUT_EXPIRES". At that point, the endpoints will select themselves as grand master and the network will not be synchronized
  * Additionally, the TSN switch is not a great choice for grandmaster as it may or may not have a connection to global time via UTC.
