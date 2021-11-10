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
#include <time.h>
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


struct timespec WAIT_DURATION = {.tv_sec = 0, .tv_nsec = 100000000};


int main(int argc, char* argv[])
{

    //configure the socket
    int rt; 
    int priority, prio_from_sock;
    int len_size = sizeof(prio_from_sock);

    srand ( time(NULL) );
    int32_t test_id = random();

    int send_sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_TSN));
    if( send_sock == -1)
    {
        printf("Send socket returned err: [%d]\n", errno);
        exit(errno);
    }   

    priority = 3;
    rt = setsockopt(send_sock, SOL_SOCKET, SO_PRIORITY, &priority, sizeof(priority));
    if (rt != 0)
    {
        printf("Failed to set priority [%d] for socket; errno: [%d]\n", priority, errno);
    }

    rt = getsockopt(send_sock, SOL_SOCKET, SO_PRIORITY, &prio_from_sock, &len_size);
            if (rt != 0)
    {
        printf("Failed to get priority [%d] ([%d] bytes) for socket; errno: [%d]\n", prio_from_sock, len_size, errno);
    } else
    {
        printf("Socket supposedly has priority [%d]\n", prio_from_sock);
    }

    struct sockaddr_ll addr;
    struct ifreq ifr;

    memset(&addr, 0, sizeof(addr));

    int eth_interface_index = get_eth_index_num(&ifr);
    if (eth_interface_index < 0)
    {
        printf("did not find a valid ethernet interface named %s", IF_NAME);
        return eth_interface_index;
    }
    printf("Ethernet interface index %d\n", eth_interface_index);
    

    addr.sll_family = AF_PACKET;
    addr.sll_protocol = htons(ETH_P_TSN);
    addr.sll_ifindex = eth_interface_index;
    addr.sll_halen = ETHER_ADDR_LEN;
    addr.sll_pkttype = PACKET_OTHERHOST;
    
    char dest_addr[ETHER_ADDR_LEN+1] = SINK_MAC_ADDR;
    char src_addr[ETHER_ADDR_LEN+1] = SOURCE_MAC_ADDR;
    memset(&(addr.sll_addr), 0, sizeof(addr.sll_addr));
    memcpy(&(addr.sll_addr), &dest_addr, ETHER_ADDR_LEN);


    //setup packets and send over ethernet
    // struct ether_tsn tsn_ethernet;
    // struct ethernet_frame_8021Q eth_frame;
    // eth_frame.TCI.tci_int = (htonl((ETH_P_VLAN << 16) | priority << 13 | VLAN_ID));
    struct ethernet_frame eth_frame;
    // memset(&eth_frame, 0, sizeof(eth_frame));
    // eth_frame.TCI.tci_int = (htonl((ETH_P_VLAN << 16) | priority << 13 | VLAN_ID));


    //recall communications typically use little-endian
    memcpy(&eth_frame.destination_mac, &dest_addr, ETHER_ADDR_LEN);
    memcpy(&eth_frame.source_mac, &src_addr, ETHER_ADDR_LEN );

    eth_frame.data_size_or_type = htons(ETH_P_TSN);
    memset(((char*)&(eth_frame.payload.data))+sizeof(struct source_sink_payload), 'q', sizeof(eth_frame.payload) - sizeof(struct source_sink_payload));
    eth_frame.payload.ss_payload.test_id = test_id;
    eth_frame.payload.ss_payload.frame_priority = priority;

    printf("**********************\nStart source side of source-sink connection for Test [%d]\n**********************\n", test_id);

    print_hex((char*) &eth_frame, 40);
    printf("\n");
    int counter = 0;
    struct timespec now;

    while(1)
    {
        // add timestamp to frame
        clock_gettime(CLOCK_REALTIME, &now);
        memcpy(&eth_frame.payload.ss_payload.tx_time, (void*) &now, sizeof(now));

        eth_frame.payload.ss_payload.frame_id = counter;
        print_hex(eth_frame.payload.data, 40); printf("\n");


        int rc = sendto(send_sock, (void*) &eth_frame, sizeof(eth_frame), 0, (struct sockaddr*) &addr, sizeof(addr));
        if (rc < 0)
        {
            printf("Socket did not send correctly... returned [%d] (error number: [%d])", rc, errno);
            // perror("socket fail");
            continue;
        }

        printf("send msg %d of  %d bytes at ", counter, rc);
        print_timespec(now);
        printf("\n");

        int no_print = 1;
        wait(WAIT_DURATION, no_print);
        fflush(stdout);
        counter++;
    }


    printf("Done\n");
}

