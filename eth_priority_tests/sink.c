/**
 * 
 * The sink will accept data from the jammer and source, although the test is if prioritized ethernet traffic (over VLAN)
 *     will have consistently lower latency than unprioritized traffic from the jammer.
 *  Expected behavior is that the prioritized traffic will not matter until the TSN switch is configured to given preference or particualr time slot
 *     to a particular VLAN and/or priority class.
 * 
 * Assumed platform: Ubuntu 20.04 LTS, Intel Nuc (series 11), NIC i225
 * 
 * Author: Reese Grimsley
 * Created: 10/29/21
 * 
 * 
 * Raw sockets references:
 *  https://www.binarytides.com/raw-sockets-c-code-linux/
 */

#include <stdio.h>
#include <sys/socket.h>
#include <errno.h>

#include "constants.h"


int main(int argc, char* argv[])
{

    int send_sock = socket(AF_PACKET,SOCK_RAW,IPPROTO_RAW);
    if( send_sock == -1)
    {
        printf("Send socket returned err: [%d]", errno)
    }

}

