
#ifndef __ETH_TEST_TYPES__
#define __ETH_TEST_TYPES__

struct ethernet_msg_8021Q
{
        char destination_mac[6];
        char source_mac[6];
        char transport_protocol[2]; //first two bytes are 0x8100 == ETH_P_8021Q
        char VLAN[2]; // highest 3 bits are PCP (priority), next bit is drop-elibile indicator, and last 12 are VLAN id
        char frame_size [2]; 
        char data[1500];
};



#endif