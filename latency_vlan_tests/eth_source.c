/**
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


 * The source will inject data into the ethernet interface; we want to see what the latency (distribution)
 *      is when a jammer is running on the same network (through a TSN switch -- Hirschman BRS8TX).
 * The network traffic from the sink should be prioritized and sent through a VLAN to be given preference over the 
 *      generic traffic from the jammer
 * 
 * Assumed platform: Ubuntu 20.04 LTS, Intel Nuc (series 11), NIC i225
 *     Must be run as SUDO!
 *     MAC address of each device (sink, jammer, and source) are assumed within the constants.h file

 * 
 * Author: Reese Grimsley
 * Created: 10/29/21
 *
 * Raw sockets references:
 *  https://www.binarytides.com/raw-sockets-c-code-linux/
 * 
 * No guarantees for this software. Use as is at your own risk. This is created as a learning exercise.
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
    int modify_prio;
    struct sockaddr_ll addr;
    struct ifreq ifr;


    char dest_addr[ETHER_ADDR_LEN+1] = SINK_MAC_ADDR;
    char src_addr[ETHER_ADDR_LEN+1] = SOURCE_MAC_ADDR;

    if (argc == 2)
    {
        priority = atoi(argv[1]);
        modify_prio = 1;
    }
    else 
    {
        printf("Using default priority for this network interface, if applicable\n");
        priority = 0;
        modify_prio = 0;
    }

    srand ( time(NULL) );
    int32_t test_id = random();

    int send_sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_TSN));
    if( send_sock == -1)
    {
        printf("Send socket returned err: [%d]\n", errno);
        exit(errno);
    }   

    if (modify_prio)
    {
        rt = setsockopt(send_sock, SOL_SOCKET, SO_PRIORITY, &priority, sizeof(priority));
        if (rt != 0)
        {
            printf("Failed to set priority [%d] for socket; errno: [%d]\n", priority, errno);
        }


        rt = getsockopt(send_sock, SOL_SOCKET, SO_PRIORITY, &prio_from_sock, &len_size);
        if (rt != 0)
        {
            printf("Failed to get priority [%d] ([%d] bytes) for socket; errno: [%d]\n", prio_from_sock, len_size, errno);
        } 
        else
        {
            printf("Socket supposedly has priority [%d]\n", prio_from_sock);
        }
    }

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

    memset(&(addr.sll_addr), 0, sizeof(addr.sll_addr));
    memcpy(&(addr.sll_addr), &dest_addr, ETHER_ADDR_LEN);

    //setup packets and send over ethernet
    struct ethernet_frame eth_frame;
    memset(&eth_frame, 0, sizeof(eth_frame));

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

