//Addresses.h

#define JAMMER_NAME "tsn3"
#define JAMMER_MAC_ADDR {0x54, 0xb2, 0x03, 0xf0, 0xd4, 0x68}  //54:b2:03:f0:d4:68
#define JAMMER_IP_ADDR  "192.168.1.224"
#define SOURCE_NAME "tsn1"
#define SOURCE_MAC_ADDR {0x54, 0xb2, 0x03, 0xf0, 0xd5, 0x0b} //54:b2:03:f0:d5:0b
#define SOURCE_IP_ADDR "192.168.1.229"
#define SINK_NAME 'tsn2'
#define SINK_MAC_ADDR {0x54, 0xb2, 0x03, 0xf0, 0xd4, 0x55}  //54:b2:03:f0:d4:55
#define SINK_IP_ADDR "192.168.1.231"
#define SINK_PORT 15810

#define ETH_INTERFACE_I225 "enp87s0\0"

#define MAX_PACKET_SIZE 1500
#define MAX_UDP_PACKET_SIZE 1350

#define JAMMER_MSG "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz"
#define JAMMER_MSG_LEN  (26 * 6 + 1) 