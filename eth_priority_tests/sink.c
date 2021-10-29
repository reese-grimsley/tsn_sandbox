/**
 * 
 * The sink will accept data from the jammer and source, although the test is if prioritized ethernet traffic (over VLAN)
 *     will have consistently lower latency than unprioritized traffic from the jammer.
 *  Expected behavior is that the prioritized traffic will not matter until the TSN switch is configured to given preference or particualr time slot
 *     to a particular VLAN and/or priority class.
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
#include <pthread.h>

#include "constants.h"


void thread_recv_jammer_data()
{
    int rcv_jam_sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_802_3));
    if( rcv_jam_sock == -1)
    {
        printf("Recv-from-jammer socket returned err: [%d]\n", errno);
        exit(errno);    
    }


    pthread_exit(NULL);
}

void thread_recv_source_data()
{
    int rcv_src_sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_TSN));
    if( rcv_src_sock == -1)
    {
        printf("Recv-from-source socket returned err: [%d]\n", errno);
        exit(errno);    
    }


    pthread_exit(NULL);
}

int main(int argc, char* argv[])
{


    pthread_t recv_jammer, recv_source;

    pthread_create(&recv_jammer, NULL, (void*) thread_recv_jammer_data, NULL);
    pthread_create(&recv_source, NULL, (void*) thread_recv_source_data, NULL);

    pthread_join(&recv_jammer);
    pthread_join(&recv_source);

    printf("Exiting sink\n");

}