
#ifndef __ETH_TEST_TYPES__
#define __ETH_TEST_TYPES__

typedef struct tag_control
{
    unsigned int priority : 3;
    unsigned int drop_indicator : 1;
    unsigned int vlan_id : 12;
} tag_control_t;

struct ethernet_frame_8021Q
{
    char destination_mac[6];
    char source_mac[6];
    char transport_protocol[2]; //first two bytes are 0x8100 == ETH_P_8021Q
    tag_control_t TCI; // highest 3 bits are PCP (priority), next bit is drop-elibile indicator, and last 12 are VLAN id
    char frame_size [2]; 
    char data[1500];
};



#endif