
#ifndef __ETH_TEST_TYPES__
#define __ETH_TEST_TYPES__

typedef struct tag_control
{
    unsigned int TPID : 16;
    unsigned int priority : 3;
    unsigned int drop_indicator : 1;
    unsigned int vlan_id : 12;
} tag_control_t;

struct source_sink_payload
{
    int32_t test_id;
    int32_t frame_id;
    int32_t frame_priority;
    struct timespec tx_time;
};

union eth_payload
{
    struct source_sink_payload ss_payload;
    char data[MAX_FRAME_DATA_LEN];
};

struct ethernet_frame_8021Q
{
    char destination_mac[ETHER_ADDR_LEN];
    char source_mac[ETHER_ADDR_LEN];
    union TCI_union
    {
        tag_control_t tci_struct;
        uint32_t tci_int;
    } TCI;
    // tag_control_t TCI; // highest 3 bits are PCP (priority), next bit is drop-elibile indicator, and last 12 are VLAN id
    uint16_t data_size_or_type ; 
    uint16_t alignment_a;
    union eth_payload payload;
    
};


struct ethernet_frame
{
    char destination_mac[ETHER_ADDR_LEN];
    char source_mac[ETHER_ADDR_LEN];
    uint16_t data_size_or_type ; 
    union eth_payload payload;
};

struct ethernet_RX_frame
{
    char destination_mac[ETHER_ADDR_LEN];
    char source_mac[ETHER_ADDR_LEN];
    uint16_t data_size_or_type ; 
    uint16_t alignment_a;
    uint32_t alignment_b;
    union eth_payload payload;

};

#endif