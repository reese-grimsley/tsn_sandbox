# Virtual LAN Configuration and Setup for TSN-enabled Networks

TSN is designed to work through Virtual Local Area Networks or VLANs. VLANs were defined in IEEE 802.1Q, and the majority of TSN-related standards are amendments of this. This page describes how to configure VLANs/virtual network interfaces such that they have known traffic/priority mappings, a local IP address for that virtual interface, and the ability to be propagated into the network (thereby avoiding manual configuration for each port for each switch).

VLANs are simple in practice. In Ethernet, which we assume to be using, it encapsulates the regular ethernet frame with a few extra bytes to specify a VLAN ID (12 bits) and a priority (3 bits). In Linux, this is generally handled by creating a virtual interface with its own IP so that sending and receiving through it will add or strip this header automatically. 

To handle priorities, Linux has 16 'traffic classes', which can be mapped to the 8 possible traffic classes. By default, these are not one-to-one; see [Additional Info on VLANs for more](./info_and_errata.md#vlan).

Disclaimer: These directions are based on an Ubuntu 20.04 LTS Linux distribution, and may not work correctly on other distributions. Further, we assume the device contains an intel i225-LM. This appears to universally name the interface *enp87s0* (although if there are multiple, the last number is likely an index).

## Command Line Configuration

Most directions online are for command-line configuration. This has the advantage of being easier to setup and debug, but the disadvantage of not persisting across resets. For that, please see [File-Based Configuration](##file-based-configuration).

This configuration, along with many other mechanisms used, requires root permissions. It uses the [iproute2](http://manpages.ubuntu.com/manpages/trusty/man8/ip.8.html) tool inherent to modern Ubuntu distributions.

### Creating the VLAN interface

First, let's create a simple VLAN with default configurations by add a virtual interface based on the ethernet NIC. We'll give it a VLAN ID of '3'. The interface of the ethernet NIC is "enp87s0", and conventions suggest to append ".$VLAN_ID" to its name for the new virtual interface.

```
sudo ip link add link enp87s0 name enp87s0.3 type vlan id 3
```

### Setting Egress traffic maping

Next, we'll set the egress mapping between traffic classes and VLAN priorities. Note that you may manipulate the traffic class at the socket level with [setsockopt](https://man7.org/linux/man-pages/man2/setsockopt.2.html) and SO_PRIORITY. There is a helper function, set_socket_priority in [helpers.h](latency_vlan_tests/helpers.h) that will do this.

```
sudo ip link set enp87s0.3 type vlan egres 0:2 1:1 2:0 3:3 4:4 5:5 6:6 7:7
```

This is the default configuration, to convert traffic_class to vlan_priority. To make this VLAN send data at a higher priority than best effort be default, map traffic class 0 to something higher than 2, e.g. ```0:3 1:0 2:1 3:2 4:4 5:5 6:6 7:7```.

You may similarly setup the ingress mapping, which tells the kernel how to prioritize incoming enqueued traffic. While not configured for our usecase, you can also set how the kernel handles queueing in hardware and/or software using the [traffic class utility](https://tsn.readthedocs.io/qdiscs.html#configuring-tsn-qdiscs).

### Setting MVRP

MVRP means Multiple VLAN Registration Protocol; it allows VLAN information to be propagated through a LAN so members of the same VLAN can reach each other through switches that are not necessarily on that VLAN. This is simply a binary option: on or off.

```
sudo ip link set enp87s0.3 type vlan mvrp on
```

### Configuration with a Single Command

The above three points can also be done in a single command for ease of use. For example, creating a VLAN with ID 5 that will use MVRP and a default priority of 3:

```
sudo ip link add link enp87s0 name enp87s0.5 type vlan id 5 mvrp on egress-qos-map 0:3 1:0 2:1 3:2 4:4 5:5 6:6 7:7
```

### Providing a VLAN Address

To communicate through the VLAN, the virtual interface should be given an IP address. The following gives an IP address of 10.0.5.101 to the interface for VLAN 5 as we configured above:

```
sudo ip addr add 10.0.5.101/24 dev enp87s0.5
```

Note that the netmask (here, the first 24 bits) is important. In this instance, it is configuring those first 24 bits (the first 3 octets of the IP address) to designate the subnet and the last 8 bits to specify the device itself.

### Turning on the Virtual Interface

Finally, we need to set the interface into the 'up' state, as it is not on by default. Otherwise, we cannot send or receive anything directly through this interface:

```
sudo ip link set enp87s0.5 up
```

Replacing ```up``` with ```down``` will similarly turn the interface off.

## File-Based Configuration

File-based configuration is harder to debug, but will persist across device/network-service reboots. Ubuntu versions newer than 17.10 use [netplan](https://netplan.io/examples/) rather than /etc/network/interfaces.

Note that one alternative to netplan would be to run a shell script with the CLI configuration commands at boot

<TODO>

## Configuration the Switch

<TODO>

### Enable vlans


### enable MVRP


### Set the priority mapping (egress only)

### See the coonfigured and learned VLANs

#### See the IPs associated with VLANs