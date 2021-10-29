/**
 * 
 * The Jammer will inject traffic into a raw socket as quickly as it can.
 *      However, we're using a 2.5Gbps capable NIC, so it's unlikely it is anywhere near saturation..
 * 
 * Assumed platform: Ubuntu 20.04 LTS, Intel Nuc (series 11), NIC i225
 *    Must be run as SUDO!

 * 
 * Author: Reese Grimsley
 * Created: 10/29/21
 * 
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


int main(int argc, char* argv[])
{

    struct sockaddr_in sink_addr;
    int jammer_sock;
    char junk_data[MAX_UDP_PACKET_SIZE];


    const size_t sockaddr_struct_len = sizeof(sink_addr);
    


    jammer_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if( jammer_sock == -1)
    {
        printf("Send socket returned err: [%d]\n", errno);
        exit(errno);
    }

    sink_addr.sin_family = AF_INET;
    sink_addr.sin_port = SINK_PORT;
    sink_addr.sin_addr.s_addr = inet_addr(SINK_IP_ADDR);

    printf("Send data as fast as possible");

    memset(junk_data, 0xc7, MAX_UDP_PACKET_SIZE);

    while(1)
    {
        sendto(jammer_sock, junk_data, MAX_UDP_PACKET_SIZE, 0, (struct sockaddr*) &sink_addr, sockaddr_struct_len);
    }

}