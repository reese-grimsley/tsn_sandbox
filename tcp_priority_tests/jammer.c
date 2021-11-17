/**
 * 
 * The Jammer will inject traffic into a raw socket as quickly as it can.
 *      However, we're using a 2.5Gbps capable NIC, so it's unlikely it is anywhere near saturation..
 *      We will send through UDP, as this is the simplest thing to do
 *          However, strange cases have been observed where the switch routes traffic through a totally different
 *          interface like wireless. This will not suitably slow traffic intended for the ethernet NIC of the sink
 * 
 * Assumed platform: Ubuntu 20.04 LTS, Intel Nuc (series 11), NIC i225
 *    Must be run as SUDO!
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
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <linux/if_packet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <errno.h>

#include "helpers.h"
#include "constants.h"
#include "types.h"

#define USE_UDP 1

int go_slower = 1;
struct timespec wait_duration = {.tv_sec=0, .tv_nsec= 1000000};

// char ADDRESS_TO_JAM[ETHER_ADDR_LEN+1] = SINK_MAC_ADDR;
char ADDRESS_TO_JAM[ETHER_ADDR_LEN+1] = SOURCE_MAC_ADDR;


int setup_sock_udp(struct sockaddr_in* addr_sink, struct sockaddr_in* addr_jammer)
{
    int jammer_sock, rt;
    char junk_data[MAX_UDP_PACKET_SIZE];

    printf("Configure jammer socket for UDP\n");

    
    jammer_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if( jammer_sock == -1)
    {
        printf("Send socket returned err: [%d]\n", errno);
        exit(errno);
    }

    memset(addr_sink, 0, sizeof(*addr_sink));
    memset(addr_jammer, 0, sizeof(*addr_jammer));


    addr_sink->sin_family = AF_INET;
    addr_sink->sin_port = SINK_PORT;
    addr_sink->sin_addr.s_addr = inet_addr(SINK_IP_ADDR_VLAN);

    addr_jammer->sin_family = AF_INET;
    addr_jammer->sin_port = JAMMER_PORT;
    addr_jammer->sin_addr.s_addr = inet_addr(JAMMER_IP_ADDR_VLAN);

    rt = bind(jammer_sock, (struct sockaddr*) &addr_jammer, sizeof(addr_jammer));
    if (rt != 0)	
    {
		perror("bind socket");
		shutdown(jammer_sock,2);
		exit(errno);
	}

    return jammer_sock;

}

int setup_sock_eth(struct sockaddr_ll* addr, struct ifreq* ifr)
{
    int jammer_sock;
    printf("Configure jammer socket for raw ethernet\n");

    jammer_sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_JAMMER));
    if(jammer_sock == -1)
    {
        printf("Send socket returned err: [%d]\n", errno);
        exit(errno);
    }  

    memset(addr, 0, sizeof(struct sockaddr_ll));

    int eth_interface_index = get_eth_index_num(ifr);
    if (eth_interface_index < 0)
    {
        printf("did not find a valid ethernet interface named %s", IF_NAME);
        return eth_interface_index;
    }
    printf("Ethernet interface index %d\n", eth_interface_index);
    
    addr->sll_family = AF_PACKET;
    addr->sll_protocol = htons(ETH_P_JAMMER);
    addr->sll_ifindex = eth_interface_index;
    addr->sll_halen = ETHER_ADDR_LEN;
    addr->sll_pkttype = PACKET_OTHERHOST;

    return jammer_sock;
}

int main(int argc, char* argv[])
{
    int jammer_sock;

    struct sockaddr_in addr_sink, addr_jammer;
    char junk_data[MAX_UDP_PACKET_SIZE];

    struct sockaddr_ll addr;
    struct ifreq ifr;
    struct ethernet_frame eth_frame;
    char dest_addr[ETHER_ADDR_LEN+1] = SINK_MAC_ADDR;
    char src_addr[ETHER_ADDR_LEN+1] = JAMMER_MAC_ADDR;

    // memcpy(&dest_addr, &ADDRESS_TO_JAM, sizeof(dest_addr));

    printf("Start jammer\n");
    
#if USE_UDP
    jammer_sock = setup_sock_udp(&addr_sink, &addr_jammer);
    memset(junk_data, '^', MAX_UDP_PACKET_SIZE);

#else
    jammer_sock = setup_sock_eth(&addr, &ifr);
    memset(&(addr.sll_addr), 0, sizeof(addr.sll_addr));
    memcpy(&(addr.sll_addr), &dest_addr, ETHER_ADDR_LEN);
    memset(((char*) &(eth_frame.payload.data)), '^', sizeof(eth_frame.payload));
    memcpy(&eth_frame.destination_mac, &dest_addr, ETHER_ADDR_LEN);
    memcpy(&eth_frame.source_mac, &src_addr, ETHER_ADDR_LEN );
    eth_frame.data_size_or_type = htons(ETH_P_JAMMER);
    print_hex((char*)&eth_frame, 48);
#endif

    printf("Send data as fast as possible\n");


    while(1)
    {
#if USE_UDP
        int rc = sendto(jammer_sock, junk_data, MAX_UDP_PACKET_SIZE, 0, (struct sockaddr*) &addr_sink, sizeof(addr_sink));
#else
        int rc = sendto(jammer_sock, (void*) &eth_frame, sizeof(eth_frame), 0, (struct sockaddr*) &addr, sizeof(addr));
#endif
        if (rc < 0)
        {
            printf("Socket did not send correctly... returned [%d] (error number: [%d])", rc, errno);
            // perror("socket fail");
            continue;
        }

        if (go_slower)
        {
            wait(wait_duration, 1);

        }

    }
    printf("Done\n");

}