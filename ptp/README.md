# Precision Time Protocol Configuration

Precision Time Protocol (PTP) is an IEEE Standard (1588) for high precision time synchronization in local area networks. Synchronization in the sub-microsecond regime is feasible, and for a small network, may even be in the 10s of nanoseconds.

Timing messages are propagated throughout the network, and messages are timestamped by hardware upon arrival to remove as much uncertainty in the timestamps as possible. This requires a NIC capable of hardware timestamping, which anything 'TSN-compatible' should be able to do.

The subdirectories here include configurations for PTP (master and client) and gPTP. It is recommended to use the gPTP (aka 802.1AS aka 'generic' PTP) scripts, as the TSN switch had trouble achieving acceptable synchrony with PTP itself. gPTP is simply a subset of PTP; PTP has a dizzying number of configuration parameters, and gPTP exists to simplify this. **The ptp_master and ptp_client configurations are left in for sake of completeness, but are not recommended**.

As described in the [Errata](../info_and_errata.md#errata), it is not recommended to use the switch as the grand master clock. It is a better idea to designate one or more of the endpoints as the master by setting the priorities to be higher (lower value) than what the switch uses.

## Synchronization and ptp4l

In Linux, the [ptp4l](https://linux.die.net/man/8/ptp4l) utility (PTP for Linux) will handle synchronization according to a [configuration file](https://github.com/openil/linuxptp/tree/master/configs).

The .conf and .cfg files provide the configuration options, as there are far too many parameters to provide directly in the command line. Although unspecified, ptp4l should automatically determine the ethernet NIC as its interface of choice, although that may require explicit selection in the configuration file is multiple are present.

## System Clock and NIC Clock with phc2sys

PTP will synchronize the clocks within the network interface. However, the system clock within Linux doesx not track this by default. The two have to be linked, typically with the [phc2sys utility](https://manpages.ubuntu.com/manpages/focal/en/man8/phc2sys.8.html).

The grand master's NIC clock should be based on that device's system clock, and every other device's system clock should be based on their NIC clock. This means the phc2sys configuration must be aware of whether the device is the grand master clock or not.

PTP clocks are nominally linked to International Atomic Time (TAI), rather than Universal Coordinated Time (UTC). The two differ by the number of [Leap Seconds](https://en.wikipedia.org/wiki/Leap_second) that have accumulated since UTC was created. As of Dec. 2021, UTC lags behind TAI by 37 seconds. This offset means that the phc2sys configuration should know the number of leap seconds and introduce an offset accordingly (a positive offset in the grand master; negative in the clients).
