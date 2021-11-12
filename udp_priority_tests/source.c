/**
 * 
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

int default_priority = 3;

int main(int argc, char* argv[])
{
    int rt; 
    int priority, prio_from_sock;
    int len_size;
    char if_name[32] = IF_NAME;

    priority = default_priority;

    if (argc == 2)
    {
        int prio = atoi(argv[1]);
        if (prio >= 0 && prio <= 7) priority = prio;
    }


    srand ( time(NULL) );
    int32_t test_id = random();

    int send_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if( send_sock == -1)
    {
        printf("Send socket returned err: [%d]\n", errno);
        exit(errno);
    }   

    priority = 3;
    rt = setsockopt(send_sock, SOL_SOCKET, SO_PRIORITY, &priority, sizeof(priority));
    if (rt != 0)
    {
        printf("Failed to set priority [%d] for socket; errno: [%d]\n", priority, errno);
    }

    len_size = sizeof(prio_from_sock);
    rt = getsockopt(send_sock, SOL_SOCKET, SO_PRIORITY, &prio_from_sock, &len_size);
    if (rt != 0)
    {
        printf("Failed to get priority [%d] ([%d] bytes) for socket; errno: [%d]\n", prio_from_sock, len_size, errno);
    } else
    {
        printf("Socket said to have priority [%d]\n", prio_from_sock);
    }

    struct sockaddr_in addr;
    struct ifreq ifr;

    memset(&addr, 0, sizeof(addr));

    // rt = get_eth_index_num(&ifr);
    // if (rt == -1)
    // {
    //     printf("Failed to get ethernet interface index number; shutdown. errno [%d]", errno);
    //     shutdown(send_sock, 2);
    //     exit(errno);
    // }
    // printf("Using network interface %d\n", ifr.ifr_ifindex);

    // if (setsockopt(send_sock, SOL_SOCKET, SO_BINDTODEVICE, if_name, sizeof(if_name)) == -1)	{
	// 	perror("SO_BINDTODEVICE");
	// 	shutdown(send_sock,2);
	// 	exit(errno);
	// }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(SINK_PORT);
    addr.sin_addr.s_addr = inet_addr(SINK_IP_ADDR);

    // addr.sll_ifindex = ifr.ifr_ifindex;

	// if (inet_aton(SINK_IP_ADDR , &addr.sin_addr) == 0) 
	// {
	// 	fprintf(stderr, "inet_aton() failed\n");
	// 	exit(1);
	// }

    union udp_dgram dgram;

    dgram.ss_payload.test_id = test_id;
    dgram.ss_payload.frame_priority = priority;
    memset(((char*)&(dgram.data))+sizeof(struct source_sink_payload), 'q', sizeof(dgram) - sizeof(struct source_sink_payload));

    printf("**********************\nStart source side of source-sink connection for Test [%d]\n**********************\n", test_id);


    int counter = 0;
    struct timespec now;

    while(1)
    {
        // add timestamp to frame
        clock_gettime(CLOCK_REALTIME, &now);
        memcpy(&dgram.ss_payload.tx_time, (void*) &now, sizeof(now));

        dgram.ss_payload.frame_id = counter;
        print_hex(dgram.data, 40); printf("\n");


        int rc = sendto(send_sock, (void*) &dgram, sizeof(dgram), 0, (struct sockaddr*) &addr, sizeof(addr));
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

