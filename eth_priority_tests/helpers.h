/**
 * Author: Reese Grimsley
 * Created on 10/29/21
 * 
 */ 
#ifndef __ETH_HELPERS__
#define __ETH_HELPERS__

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

#include "constants.h"
#include "types.h"

#define min(a,b) (((a) < (b)) ? (a) : (b))
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define MIN min
#define MAX max


int get_num_leapseconds(void); // doesn't appear to work; returns 0. To really be accurate, probably needs a connection to an NTP server of similar, which an NTP client like chrony will update according (i.e., tai_offset will be updated)
int configure_hw_timestamping(int sock_fd);
int get_hw_timestamp_from_msg(struct msghdr* msg, struct timespec* ts);
int get_eth_index_num(struct ifreq* ifr);

void print_timespec(const struct timespec ts);
void time_diff(const struct timespec * last_time, const struct timespec * current_time, struct timespec* diff);
int wait_until(struct timespec ts, int no_print);
int wait(struct timespec sleep_duration, int no_print);

void print_hex(const char* msg, int len);

int write_timespec_to_csv(FILE* f, const struct timespec ts, int32_t frame_id, int32_t test_id, int32_t priority)


#endif