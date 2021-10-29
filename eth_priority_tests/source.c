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


int main(int argc, char* argv[])
{

    //configure the socket
    int send_sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_TSN));
    if( send_sock == -1)
    {
        printf("Send socket returned err: [%d]\n", errno);
        exit(errno);
    }


}

