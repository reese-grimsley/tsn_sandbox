/*
MIT License

Copyright (c) 2021 Reese Grimsley

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/
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
    uint16_t alignment_a;

    union eth_payload payload;
};

struct ethernet_RX_frame
{
    char destination_mac[ETHER_ADDR_LEN];
    char source_mac[ETHER_ADDR_LEN];
    uint16_t data_size_or_type ; 
    char padding[6]; //ugly fix. Depends on the compiler/host. NUC host aligns to 64bit word, which mucks with the discarded VLAN header. Have to play nasty tricks to make the payload come out correctly. This is only necessary if the frame is manually configured with a VLAN header (802.1Q) that the NIC in the receiver strips. If VLAN tag is auto-added on egress by NIC, then just use ethernet_frame
    union eth_payload payload;

};

union udp_dgram
{
    struct source_sink_payload ss_payload;
    char data[MAX_UDP_PACKET_SIZE];
};

union tcp_packet
{
    struct source_sink_payload ss_payload;
    char data[MAX_TCP_PACKET_SIZE];
};

#endif