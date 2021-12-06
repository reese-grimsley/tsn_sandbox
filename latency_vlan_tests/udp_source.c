/**
 MIT License

Copyright (c) 2021 Reese Grimsley

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

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


struct timespec WAIT_DURATION = {.tv_sec = 1, .tv_nsec = 00000000};

int default_priority = 0;

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
        printf("Passed arg %s; intepreted as priority [%d]\n", argv[1], prio);
        if (prio >= 0 && prio <= 7) priority = prio;
    }


    srand ( time(NULL) );
    int32_t test_id = random();

    int send_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if( send_sock == -1)
    {
        printf("Send socket returned err: [%d]\n", errno);
        exit(errno);
    }   

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

    if (setsockopt(send_sock, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
    {
        printf("setsockopt(SO_REUSEADDR) failed");
        shutdown(send_sock, 2);
        exit(errno);
    }

    struct sockaddr_in addr_sink, addr_src;
    struct ifreq ifr;

    memset(&addr_sink, 0, sizeof(addr_sink));
    memset(&addr_src, 0, sizeof(addr_src));

    addr_sink.sin_family = AF_INET;
    addr_sink.sin_port = htons(SINK_PORT);
    addr_sink.sin_addr.s_addr = inet_addr(SINK_IP_ADDR_VLAN);

    addr_src.sin_family = AF_INET;
    addr_src.sin_port = htons(SINK_PORT);
    addr_src.sin_addr.s_addr = inet_addr(SOURCE_IP_ADDR_VLAN);

    // addr_sink.sll_ifindex = ifr.ifr_ifindex;

	// if (inet_aton(SINK_IP_ADDR , &addr_sink.sin_addr) == 0) 
	// {
	// 	fprintf(stderr, "inet_aton() failed\n");
	// 	exit(1);
	// }

    //bind the source to the IP so it uses the VLAN we want
    rt = bind(send_sock, (struct sockaddr*) &addr_src, sizeof(addr_src));
    if (rt != 0)	
    {
		perror("bind socket");
		shutdown(send_sock,2);
		exit(errno);
	}
    printf("Bound source to VLAN address %s\n", SOURCE_IP_ADDR_VLAN);

    rt = connect(send_sock, (struct sockaddr*) &addr_sink, sizeof(addr_sink));
    if (rt != 0)	
    {
		perror("connect socket");
		shutdown(send_sock,2);
		exit(errno);
	}
    printf("Connected source to sink at VLAN address %s\n", SINK_IP_ADDR_VLAN);


    union tcp_packet pkt;

    pkt.ss_payload.test_id = test_id;
    pkt.ss_payload.frame_priority = priority;
    memset(((char*)&(pkt.data))+sizeof(struct source_sink_payload), 'q', sizeof(pkt) - sizeof(struct source_sink_payload));

    printf("**********************\nStart source side of source-sink connection for Test [%d]\n**********************\n", test_id);


    int counter = 0;
    struct timespec now;

    while(1)
    {
        // add timestamp to frame
        clock_gettime(CLOCK_REALTIME, &now);
        memcpy(&pkt.ss_payload.tx_time, (void*) &now, sizeof(now));

        pkt.ss_payload.frame_id = counter;
        // print_hex(pkt.data, 40); printf("\n");


        int rc = sendto(send_sock, (void*) &pkt, sizeof(pkt), 0, (struct sockaddr*) &addr_sink, sizeof(addr_sink));
        if (rc < 0)
        {
            printf("Socket did not send correctly... returned [%d] (error number: [%d])", rc, errno);
            // perror("socket fail");
            continue;
        }

        printf("send msg %d of %d bytes at ", counter, rc);
        print_timespec(now);
        printf("\n");

        char recv_data[32];
        memset(recv_data, 0, 32);
        recv(send_sock, (void*)recv_data, 32, 0);
        printf("Recived string from sink: [%s]\n", recv_data);

        int no_print = 1;
        wait(WAIT_DURATION, no_print);
        
        fflush(stdout);
        counter++;


    }


    printf("Done\n");
}

