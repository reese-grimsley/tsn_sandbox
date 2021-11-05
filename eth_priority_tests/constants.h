//Addresses.h

#define JAMMER_NAME "tsn3"
#define JAMMER_MAC_ADDR {0x54, 0xb2, 0x03, 0xf0, 0xd4, 0x68}  //54:b2:03:f0:d4:68
#define JAMMER_IP_ADDR  "192.168.1.224"
#define SOURCE_NAME "tsn1"
#define SOURCE_MAC_ADDR {0x54, 0xb2, 0x03, 0xf0, 0xd4, 0x55}  //54:b2:03:f0:d4:55
#define SOURCE_IP_ADDR "192.168.1.229"
#define SINK_NAME 'tsn2'
#define SINK_MAC_ADDR {0x54, 0xb2, 0x03, 0xf0, 0xd5, 0x0b}  //54:b2:03:f0:d5:0b
#define SINK_IP_ADDR "192.168.1.231"
#define SINK_PORT 15810

#define ETH_INTERFACE_I225 "enp87s0\0"
#define ETH_INTERFACE_I225_VLAN3 "enp87s0.3\0"
#define IF_NAME ETH_INTERFACE_I225

#define VLAN_ID 3

#define MAX_FRAME_SIZE 1500
#define MAX_FRAME_DATA_LEN 1350
#define MAX_UDP_PACKET_SIZE 1350



//From openAVNU; does not seem to correspond to any well-known types of ethernet message/protocol.
// https://github.com/torvalds/linux/blob/master/include/uapi/linux/if_ether.h
#define MVRP_ETYPE 0x88F5 //this one exists in the kernel
#define MSRP_ETYPE 0x22EA // close to ETH_TSN
#define MMRP_ETYPE 0x88F6
#define ETH_P_VLAN ETH_P_8021Q // or ETH_P_8021AD?