#define SOURCE_NAME "tsn1"
#define SOURCE_MAC_ADDR {0x54, 0xb2, 0x03, 0xf0, 0xd4, 0x55}  //54:b2:03:f0:d4:55
#define SOURCE_IP_ADDR "192.168.1.229"
#define SOURCE_IP_ADDR_VLAN "10.0.1.101"
#define SINK_NAME 'tsn2'
#define SINK_MAC_ADDR {0x54, 0xb2, 0x03, 0xf0, 0xd5, 0x0b}  //54:b2:03:f0:d5:0b
#define SINK_IP_ADDR "192.168.1.231"
#define SINK_IP_ADDR_VLAN "10.0.1.102"
#define SINK_PORT 15810
#define JAMMER_NAME "tsn3"
#define JAMMER_MAC_ADDR {0x54, 0xb2, 0x03, 0xf0, 0xd4, 0x68}  //54:b2:03:f0:d4:68
#define JAMMER_IP_ADDR  "192.168.1.224"
#define JAMMER_IP_ADDR_VLAN "10.0.1.103"

#define ETH_INTERFACE_I225 "enp87s0\0"
#define ETH_INTERFACE_I225_VLAN3 "enp87s0.3\0"
#define IF_NAME ETH_INTERFACE_I225_VLAN3

#define VLAN_ID 3

#define MAX_FRAME_SIZE 1500
#define MAX_FRAME_DATA_LEN 1360
#define MAX_UDP_PACKET_SIZE 1360
#define MAX_TCP_PACKET_SIZE 150

//37 as of Nov. 5 2021. This will change, but not until at least June 30/2022. Should be updated or read from the system. NICs are sync'd to TAI which is LEAP_SECONDS_FFSET ahead of UTC, which the RTC/CLOCK_REALTIME reads.
#define LEAP_SECONDS_OFFSET 37 

//From openAVNU; does not seem to correspond to any well-known types of ethernet message/protocol.
// https://github.com/torvalds/linux/blob/master/include/uapi/linux/if_ether.h
#define MVRP_ETYPE 0x88F5 //this one exists in the kernel
#define MSRP_ETYPE 0x22EA // close to ETH_TSN (0x22f0)
#define MMRP_ETYPE 0x88F6
#define ETH_P_VLAN ETH_P_8021Q // or ETH_P_8021AD?
#define ETH_P_JAMMER 0x89FF // something unique that will not conflict with other frame type

#define LATENCY_SAMPLES_TO_LOG 1000