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

    if (configure_hw_timestamping(rcv_jam_sock) != 0)
    {
        printf("failed to setup timestamping on recv\n");
        pthread_exit(NULL);
    }

    jammer_recv_addr.sin_family = AF_INET;
    jammer_recv_addr.sin_port = SINK_PORT;
    jammer_recv_addr.sin_addr.s_addr = inet_addr(SINK_IP_ADDR);


    bind(rcv_jam_sock, (struct sockaddr*) &jammer_recv_addr, sizeof(jammer_recv_addr));

    char ctrl[4096];
    struct cmsghdr *cmsg = (struct cmsghdr *) &ctrl;

    msg.msg_control = (char *) ctrl;
    msg.msg_controllen = sizeof(ctrl);

    msg.msg_name = &jammer_recv_addr;
    msg.msg_namelen = sizeof(jammer_recv_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    iov.iov_base = recv_data;
    iov.iov_len = MAX_UDP_PACKET_SIZE;

    // struct timespec ts;
    int level, type, count = 1;

    while(1)
    {
        // recvfrom(rcv_jam_sock, recv_data, MAX_UDP_PACKET_SIZE, 0, (struct sockaddr*) &jammer_send_addr, &sizeof_send_addr);
        recvmsg(rcv_jam_sock, &msg, 0);
        // printf("recv pkt [%d]\n", count);


        int level, type;
        struct timespec ts;
        if (get_hw_timestamp_from_msg(&msg, &ts))
        {
            // printf("TIMESTAMP %ld.%09ld\n", (long)ts.tv_sec, (long)ts.tv_nsec);

        }
        count++;
    }

    shutdown(rcv_jam_sock, 2);
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

    struct ifreq ifr;
    int rc;
    char ctrl[4096], data[4096], buf[4096];
    struct cmsghdr *cmsg = (struct cmsghdr *) &ctrl;
    struct sockaddr_ll rcv_src_addr;
    struct timespec ts;
    struct msghdr msg;
    struct iovec iov;

    memset(data, 0, 4096);
    iov.iov_base = data;
    iov.iov_len = 4096;

    msg.msg_control = (char *) ctrl;
    msg.msg_controllen = sizeof(ctrl);

    msg.msg_name = &rcv_src_addr;
    msg.msg_namelen = sizeof(rcv_src_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    int rcv_src_sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_TSN));
    // int rcv_src_sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_VLAN));
    // int rcv_src_sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if( rcv_src_sock == -1)
    {
        printf("Recv-from-source socket returned err: [%d]\n", errno);
        exit(errno);    
    }

    rc = get_eth_index_num(&ifr);
    if (rc == -1)
    {
        printf("Failed to get ethernet interface index number; shutdown. errno [%d]", errno);
        shutdown(rcv_src_sock, 2);
        exit(errno);
    }
    printf("Using network interface %d\n", ifr.ifr_ifindex);

    rc = configure_hw_timestamping(rcv_src_sock);
    if (rc == -1)
    {
        printf("Failed to setup; shutdown. errno [%d]", errno);
        shutdown(rcv_src_sock, 2);
        exit(errno);

    }

    rcv_src_addr.sll_family = AF_PACKET;
    // rcv_src_addr.sll_protocol = htons(ETH_P_ALL);
    // rcv_src_addr.sll_protocol = htons(ETH_P_VLAN);
    rcv_src_addr.sll_protocol = htons(ETH_P_TSN);
    rcv_src_addr.sll_ifindex = ifr.ifr_ifindex;
    rcv_src_addr.sll_halen = ETHER_ADDR_LEN;
    rcv_src_addr.sll_pkttype = PACKET_OTHERHOST;

    char if_name[20] = IF_NAME;
    if (setsockopt(rcv_src_sock, SOL_SOCKET, SO_BINDTODEVICE, if_name, sizeof(if_name)) == -1)	{
		perror("SO_BINDTODEVICE");
		shutdown(rcv_src_sock,2);
		exit(errno);
	}

    char dest_addr[ETHER_ADDR_LEN+1] = SINK_MAC_ADDR;
    memset(&(rcv_src_addr.sll_addr), 0, sizeof(rcv_src_addr.sll_addr));
    memcpy(&(rcv_src_addr.sll_addr), &dest_addr, ETHER_ADDR_LEN);

    printf("Start steady state in sink of source-sink connection\n");
    struct timespec now, start, diff;
    struct timespec time_from_source, time_from_nic, t_prop;

    clock_gettime(CLOCK_REALTIME, &start);
    printf("Started steady state at t=");
    print_timespec(start);
    printf("\n");
    fflush(stdout);

    int tsn_msgs_received = 0, last_frame_id = -1;
    FILE* log_file = fopen("source_sink_latency.log", "a");
    //TODO: open data csv file
    while(tsn_msgs_received < LATENCY_SAMPLES_TO_LOG) 
    {
        int msg_size;
        msg_size = recvmsg(rcv_src_sock, &msg, 0);
        // clock_gettime(CLOCK_REALTIME, &now);
        if (msg_size == -1 || (((struct sockaddr_ll*) msg.msg_name)->sll_protocol) == 0x0008) //also ignore IP
        {
            // printf("recvmsg signalled error: [%d]\n", errno);
        }
        else
        {
            printf("Received message of length [%d]\n", msg_size);
            
            //if this came within a 802.1Q frame, the offsets will need to change to account for ethernet addresses
            // printf("receive message with protocol: %04x\n",((struct sockaddr_ll*) msg.msg_name)->sll_protocol);//can filter based on this as well..

            if ( (((struct sockaddr_ll*) msg.msg_name)->sll_protocol) == htons(ETH_P_TSN) )
            {
                struct ethernet_RX_frame frame;
                int32_t frame_id, priority, test_id;
                tsn_msgs_received++;
                //this is a frame we want.
                printf("[%d]th TSN frame!\n", tsn_msgs_received);
                memcpy(&frame, msg.msg_iov->iov_base, min(sizeof(frame), msg.msg_iov->iov_len));
                print_hex((char*) &frame, 40); printf("\n");
                print_hex(frame.payload.data+4, 40); printf("\n");

                memcpy(&time_from_source, &(frame.payload.ss_payload.tx_time), sizeof(struct timespec));

                frame_id = frame.payload.ss_payload.frame_id;
                priority = frame.payload.ss_payload.frame_priority;
                test_id = frame.payload.ss_payload.test_id;

                if (get_hw_timestamp_from_msg(&msg, &time_from_nic))
                {


                    memset(&t_prop, 0, sizeof(t_prop));
                    time_diff(&time_from_source, &time_from_nic, &t_prop);
                    t_prop.tv_sec -= MAX(get_num_leapseconds(), LEAP_SECONDS_OFFSET);
                    printf("Propagation time (NIC, corrected for UTC): ");
                    print_timespec(t_prop);
                    printf("\n-----\n");

                    if (last_frame_id != -1 && last_frame_id > frame_id)
                    {
                        fclose(log_file);
                        break;
                    }

                    //TODO: add statistics and/or file-write
                    write_timespec_to_csv(log_file, t_prop, frame_id, test_id, priority);

                    //break if counter from frame < current one held here.       
                }
                last_frame_id = frame_id;
                if (tsn_msgs_received % 50 == 0) fflush(log_file);

            }

            fflush(stdout);
            break;
        }

    }
    printf("Exiting sink's main loop; release files and resources...\n");

    fclose(log_file);

    printf("Done!\n");
    pthread_exit(NULL);
}

int main(int argc, char* argv[])
{
    int use_jammer = 0;
    if (argc >=2 && strcmp(argv[1], "jam") == 0 )
    {
        use_jammer = 1;
    }

    pthread_t recv_jammer, recv_source;

    if (use_jammer)
    {
        pthread_create(&recv_jammer, NULL, (void*) thread_recv_jammer_with_timestamping, NULL);
    }
    pthread_create(&recv_source, NULL, (void*) thread_recv_source_data, NULL);

    if (use_jammer)
    {
        pthread_join(recv_jammer, NULL);
    }
    pthread_join(recv_source, NULL);

    printf("Exiting sink\n");

}
