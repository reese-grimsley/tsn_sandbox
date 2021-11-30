/**
 * 
 * The Jammer will inject traffic into a raw socket as quickly as it can.
 *      However, we're using a 2.5Gbps capable NIC, so it's unlikely it is anywhere near saturation..
 *      We will send through UDP or ethernet
 *          However, strange cases have been observed where the switch routes traffic through a totally different
 *          interface like wireless. This will not suitably slow traffic intended for the ethernet NIC of the sink
 * 
 * Assumed platform: Ubuntu 20.04 LTS, Intel Nuc (series 11), NIC i225
 *    Must be run as SUDO!
 *     MAC address of each device (sink, jammer, and source) are assumed within the constants.h file
 * 
 * Author: Reese Grimsley
 * Created: 11/15/21
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


int go_slower = 0; //boolean flag to wait between outgoing messages
struct timespec wait_duration = {.tv_sec=0, .tv_nsec= 1000};

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
    addr_sink->sin_port = htons(SINK_PORT+1);
    addr_sink->sin_addr.s_addr = inet_addr(SINK_IP_ADDR_VLAN);

    addr_jammer->sin_family = AF_INET;
    addr_jammer->sin_port = htons(JAMMER_PORT);
    addr_jammer->sin_addr.s_addr = inet_addr(JAMMER_IP_ADDR_VLAN);

    rt = bind(jammer_sock, (struct sockaddr*) addr_jammer, sizeof(*addr_jammer));
    if (rt != 0)	
    {
		perror("bind socket");
		shutdown(jammer_sock,2);
		exit(errno);
	}

    return jammer_sock;

}

int main(int argc, char* argv[])
{
    int jammer_sock;

    struct sockaddr_in addr_sink, addr_jammer;
    char junk_data[MAX_UDP_PACKET_SIZE];

    printf("Start jammer\n");
    
    jammer_sock = setup_sock_udp(&addr_sink, &addr_jammer);
    memset(junk_data, '^', MAX_UDP_PACKET_SIZE);


    printf("Send data as fast as possible\n");


    while(1)
    {
        int rc = sendto(jammer_sock, junk_data, MAX_UDP_PACKET_SIZE, 0, (struct sockaddr*) &addr_sink, sizeof(addr_sink));

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