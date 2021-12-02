# Virtual LAN Configuration and Setup for TSN-enabled Networks

TSN is designed to work through Virtual Local Area Networks or VLANs. VLANs were defined in IEEE 802.1Q, and the majority of TSN-related standards are amendments of this. This page describes how to configure VLANs/virtual network interfaces such that they have known traffic/priority mappings, a local IP address for that virtual interface, and the ability to be propagated into the network (thereby avoiding manual configuration for each port for each switch).

Disclaimer: These directions are based on an Ubuntu 20.04 LTS Linux distribution, and may not work correctly on other distributions

## Command Line Configuration

Most directions online are for command-line configuration. This has the advantage of being easier to setup and debug, but the disadvantage of not persisting across resets. For that, please see [File-Based Configuration](##File-Based Configuration)


## File-Based Configuration