/**
 * 
 * The source will inject data into the ethernet interface; we want to see what the latency (distribution)
 *      is when a jammer is running on the same network (through a TSN switch -- Hirschman BRS8TX).
 * The network traffic from the sink should be prioritized and sent through a VLAN to be given preference over the 
 *      generic traffic from the jammer
 * 
 * Assumed platform: Ubuntu 20.04 LTS, Intel Nuc (series 11), NIC i225
 *     Must be run as SUDO!
 * 
 * Author: Reese Grimsley
 * Created: 10/29/21
 *
 * Raw sockets references:
 *  https://www.binarytides.com/raw-sockets-c-code-linux/
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <linux/if_packet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <errno.h>

#include "constants.h"
#include "helpers.h"
#include "types.h"


struct timespec WAIT_DURATION = {.tv_sec = 0, .tv_nsec = 500000000};


int main(int argc, char* argv[])
{

    //configure the socket
    int send_sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_8021Q));
    if( send_sock == -1)
    {
        printf("Send socket returned err: [%d]\n", errno);
        exit(errno);
    }   

    struct sockaddr_ll addr;
    struct ifreq ifr;

    memset(&addr, 0, sizeof(addr));

    int eth_interface_index = get_eth_index_num(&ifr);
    if (eth_interface_index < 0)
    {
        printf("did not find a valid ethernet interface named %s", ETH_INTERFACE_I225);
        return eth_interface_index;
    }
    

    addr.sll_family = AF_PACKET;
    addr.sll_protocol = htons(ETH_P_VLAN);
    addr.sll_ifindex = eth_interface_index;
    addr.sll_halen = ETHER_ADDR_LEN;
    addr.sll_pkttype = PACKET_OTHERHOST;
    
    char dest_addr[ETHER_ADDR_LEN+1] = SINK_MAC_ADDR;
    char src_addr[ETHER_ADDR_LEN+1] = SOURCE_MAC_ADDR;
    memset(&(addr.sll_addr), 0, sizeof(addr.sll_addr));
    memcpy(&(addr.sll_addr), &dest_addr, ETHER_ADDR_LEN);


    //setup packets and send over ethernet
    // struct ether_tsn tsn_ethernet;
    struct ethernet_frame_8021Q eth_frame;

    //recall communications typically use little-endian
    memcpy(&eth_frame.destination_mac, &dest_addr, ETHER_ADDR_LEN);
    memcpy(&eth_frame.source_mac, &src_addr, ETHER_ADDR_LEN );
    eth_frame.transport_protocol[0] = 0x00;
    eth_frame.transport_protocol[1] = 0x81; //little-endian
    eth_frame.TCI.priority = 0;
    eth_frame.TCI.drop_indicator = 0; 
    eth_frame.TCI.vlan_id = 0; //0 is null/void -- non-zero VLAN needs to be configured into the switch 
    eth_frame.data_size = MAX_FRAME_DATA_LEN;
    memset(&eth_frame.data, 'q', MAX_FRAME_DATA_LEN);

    while(1)
    {

        int rc = sendto(send_sock, (void*) &eth_frame, sizeof(eth_frame), 0, (struct sockaddr*) &addr, sizeof(addr));
        if (rc < 0)
        {
            printf("Socket did not send correctly... returned [%d] (error number: [%d])", rc, errno);
            perror("socket fail");
        }
        int no_print = 1;
        wait(WAIT_DURATION, no_print);

    }



}

