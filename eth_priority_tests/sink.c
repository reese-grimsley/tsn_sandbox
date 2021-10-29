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
#include <linux/sockios.h>
#include <linux/net_tstamp.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <errno.h>
#include <pthread.h>

#include "constants.h"

int setup_timestamp_on_rx_udp(int sock)
{
    int flags;
    flags   = SOF_TIMESTAMPING_RX_SOFTWARE
            | SOF_TIMESTAMPING_RX_HARDWARE 
            | SOF_TIMESTAMPING_RAW_HARDWARE;
    if (setsockopt(sock, SOL_SOCKET, SO_TIMESTAMPING, &flags, sizeof(flags)) < 0)
    {
        printf("ERROR: setsockopt SO_TIMESTAMPING: [%d]\n", errno);
        return errno;
    }


}

void thread_recv_jammer_with_timestamping()
{
    struct msghdr msg;
    struct iovec iov;
    char recv_data[MAX_UDP_PACKET_SIZE];
    struct sockaddr_in jammer_recv_addr, jammer_send_addr;
    socklen_t sizeof_send_addr = sizeof(jammer_send_addr);


    int rcv_jam_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if( rcv_jam_sock == -1)
    {
        printf("Recv-from-jammer socket returned err: [%d]\n", errno);
        exit(errno);    
    }
    setup_timestamp_on_rx_udp(rcv_jam_sock);

    jammer_recv_addr.sin_family = AF_INET;
    jammer_recv_addr.sin_port = SINK_PORT;
    jammer_recv_addr.sin_addr.s_addr = inet_addr(SINK_IP_ADDR);

    bind(rcv_jam_sock, (struct sockaddr*) &jammer_recv_addr, sizeof(jammer_recv_addr));

    char ctrl[CMSG_SPACE(sizeof(struct timespec))];
    struct cmsghdr *cmsg = (struct cmsghdr *) &ctrl;

    msg.msg_control = (char *) ctrl;
    msg.msg_controllen = sizeof(ctrl);

    msg.msg_name = &jammer_recv_addr;
    msg.msg_namelen = sizeof(jammer_recv_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    iov.iov_base = recv_data;
    iov.iov_len = MAX_UDP_PACKET_SIZE;

    struct timespec ts;
    int level, type;

    while(1)
    {
        // recvfrom(rcv_jam_sock, recv_data, MAX_UDP_PACKET_SIZE, 0, (struct sockaddr*) &jammer_send_addr, &sizeof_send_addr);
        recvmsg(rcv_jam_sock, &msg, 0);
        printf("recv pkt\n");


        int level, type;
        struct timespec *ts = NULL;
        for (cmsg = CMSG_FIRSTHDR(&msg); cmsg != NULL; cmsg = CMSG_NXTHDR(&msg, cmsg))
        {
            if (SOL_SOCKET == cmsg->cmsg_level && SO_TIMESTAMPING == cmsg->cmsg_type) {
                ts = (struct timespec *) CMSG_DATA(cmsg);
                printf("HW TIMESTAMP %ld.%09ld\n", (long)ts->tv_sec, (long)ts->tv_nsec);
            }
        }
    }


    pthread_exit(NULL);
}

void thread_recv_jammer_data()
{
    char recv_data[MAX_UDP_PACKET_SIZE];
    struct sockaddr_in jammer_recv_addr, jammer_send_addr;
    socklen_t sizeof_send_addr = sizeof(jammer_send_addr);


    int rcv_jam_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if( rcv_jam_sock == -1)
    {
        printf("Recv-from-jammer socket returned err: [%d]\n", errno);
        exit(errno);    
    }

    jammer_recv_addr.sin_family = AF_INET;
    jammer_recv_addr.sin_port = SINK_PORT;
    jammer_recv_addr.sin_addr.s_addr = inet_addr(SINK_IP_ADDR);

    bind(rcv_jam_sock, (struct sockaddr*) &jammer_recv_addr, sizeof(jammer_recv_addr));

    while(1)
    {
        recvfrom(rcv_jam_sock, recv_data, MAX_UDP_PACKET_SIZE, 0, (struct sockaddr*) &jammer_send_addr, &sizeof_send_addr);
        printf("recv pkt\n");
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

    pthread_create(&recv_jammer, NULL, (void*) thread_recv_jammer_with_timestamping, NULL);
    pthread_create(&recv_source, NULL, (void*) thread_recv_source_data, NULL);

    pthread_join(recv_jammer, NULL);
    pthread_join(recv_source, NULL);

    printf("Exiting sink\n");

}
