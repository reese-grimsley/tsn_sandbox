/**
 * 
 * The sink will accept data from the jammer and source, although the test is if prioritized ethernet traffic (over VLAN)
 *     will have consistently lower latency than unprioritized traffic from the jammer.
 *  Expected behavior is that the prioritized traffic will not matter until the TSN switch is configured to given preference or particualr time slot
 *     to a particular VLAN and/or priority class.
 * 
 * Assumed platform: Ubuntu 20.04 LTS, Intel Nuc (series 11), NIC i225
 *    Must be run as SUDO!
 *     MAC address of each device (sink, jammer, and source) are assumed within the constants.h file
 *
 * Author: Reese Grimsley
 * Created: 10/29/21
 * 
 * 
 * Raw sockets references:
 *  https://www.binarytides.com/raw-sockets-c-code-linux//***
 * 
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
#include <linux/sockios.h>
#include <linux/net_tstamp.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>
#include <time.h>

#include "constants.h"
#include "helpers.h"
#include "types.h"


int configure_source_receiving_sock(uint16_t frame_type, struct ifreq *ifr, struct sockaddr_ll *rcv_src_addr)
{
    int rcv_src_sock, rc;
    char dest_addr[ETHER_ADDR_LEN+1]= SINK_MAC_ADDR;
    char if_name[32] = IF_NAME;

    rcv_src_sock = socket(AF_PACKET, SOCK_RAW, htons(frame_type)); //e.g. ETH_P_TSN
    if( rcv_src_sock == -1)
    {
        printf("Recv-from-source socket returned err: [%d]\n", errno);
        exit(errno);    
    }

    rc = get_eth_index_num(ifr);
    if (rc == -1)
    {
        printf("Failed to get ethernet interface index number; shutdown. errno [%d]", errno);
        shutdown(rcv_src_sock, 2);
        exit(errno);
    }
    printf("Using network interface %d\n", ifr->ifr_ifindex);

    rc = configure_hw_timestamping(rcv_src_sock);
    if (rc == -1)
    {
        printf("Failed to setup; shutdown. errno [%d]", errno);
        shutdown(rcv_src_sock, 2);
        exit(errno);
    }

    rcv_src_addr->sll_family = AF_PACKET;
    rcv_src_addr->sll_protocol = htons(frame_type);
    rcv_src_addr->sll_ifindex = ifr->ifr_ifindex;
    rcv_src_addr->sll_halen = ETHER_ADDR_LEN;
    rcv_src_addr->sll_pkttype = PACKET_OTHERHOST;

    if (setsockopt(rcv_src_sock, SOL_SOCKET, SO_BINDTODEVICE, if_name, sizeof(if_name)) == -1)	{
		perror("SO_BINDTODEVICE");
		shutdown(rcv_src_sock,2);
		exit(errno);
	}

    memset(&(rcv_src_addr->sll_addr), 0, sizeof(rcv_src_addr->sll_addr));
    memcpy(&(rcv_src_addr->sll_addr), &dest_addr, ETHER_ADDR_LEN);

    return rcv_src_sock;
}

void thread_recv_source_data()
{

    struct ifreq ifr;
    int rc;
    char ctrl[4096], data[4096], buf[4096];
    struct cmsghdr *cmsg;
    struct sockaddr_ll rcv_src_addr;
    struct timespec ts;
    struct msghdr msg;
    struct iovec iov;
    struct timespec now, start, diff, time_from_source, time_from_nic, t_prop;
    int16_t frame_type;
    int tsn_msgs_received, last_frame_id;

    memset(data, 0, 4096);
    iov.iov_base = data;
    iov.iov_len = 4096;

    frame_type = ETH_P_TSN;
    int rcv_src_sock = configure_source_receiving_sock(frame_type, &ifr, &rcv_src_addr);
    
    // setup control messages; these are retrieved from the kernel/socket/NIC to get the hardware RX timestampß
    cmsg = (struct cmsghdr *) &ctrl;
    msg.msg_control = (char *) ctrl;
    msg.msg_controllen = sizeof(ctrl);
    msg.msg_name = &rcv_src_addr;
    msg.msg_namelen = sizeof(rcv_src_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    printf("Start steady state in sink of source-sink connection\n");

    tsn_msgs_received = 0;
    last_frame_id = -1;
    FILE* log_file = fopen("eth_source_sink_latency.csv", "a");

    clock_gettime(CLOCK_REALTIME, &start);
    printf("Started steady state at t=");
    print_timespec(start);
    printf("\n");
    fflush(stdout);

    while(tsn_msgs_received < LATENCY_SAMPLES_TO_LOG) 
    {
        int msg_size;
        msg_size = recvmsg(rcv_src_sock, &msg, 0);

        if ( (((struct sockaddr_ll*) msg.msg_name)->sll_protocol) == htons(frame_type) )
        {
            struct ethernet_frame* frame;
            union eth_payload payload;
            int32_t frame_id, priority, test_id;

            tsn_msgs_received++;

            //this is a frame we want.
            frame = (struct ethernet_frame*) msg.msg_iov->iov_base;
            payload = frame->payload;

            //retrieve data from the payload
            memcpy(&time_from_source, &(payload.ss_payload.tx_time), sizeof(struct timespec));
            frame_id = payload.ss_payload.frame_id;
            priority = payload.ss_payload.frame_priority;
            test_id = payload.ss_payload.test_id;

            printf("[%d]th TSN frame with priority [%d]!\n", tsn_msgs_received, priority);
            
            //calculate and log latency as differences between software timestamp prior to TX and hardware timestamp of RX. To get hardware TX timestamp, need 2 messages. 
            if (get_hw_timestamp_from_msg(&msg, &time_from_nic))
            {
                memset(&t_prop, 0, sizeof(t_prop));
                time_diff(&time_from_source, &time_from_nic, &t_prop);
                t_prop.tv_sec -= MAX(get_num_leapseconds(), LEAP_SECONDS_OFFSET);
                printf("Propagation time (NIC, corrected for UTC): ");
                print_timespec(t_prop);
                printf("\n-----\n");

                //keep logs consistent within the same test
                if (last_frame_id != -1 && last_frame_id > frame_id)
                {
                    fclose(log_file);
                    break;
                }

                //add statistics and/or file-write
                write_frame_time_to_csv(log_file, t_prop, frame_id, test_id, priority);

            }
            last_frame_id = frame_id;
            //ensure some data gets written in case of intermittent failure
            if (tsn_msgs_received % 50 == 0) fflush(log_file);

        }
        fflush(stdout);

    }
    printf("Exiting sink's main loop; release files and resources...\n");

    fclose(log_file);

    printf("Done!\n");
    pthread_exit(NULL);
}

int main(int argc, char* argv[])
{
    int use_jammer = 0;

    pthread_t recv_jammer, recv_source;

    pthread_create(&recv_source, NULL, (void*) thread_recv_source_data, NULL);

    pthread_join(recv_source, NULL);

    printf("Exiting sink\n");

}
