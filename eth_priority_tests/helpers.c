#include "helpers.h"

// doesn't appear to work; returns 0. To really be accurate, probably needs a connection to an NTP server of similar, which an NTP client like chrony will update according (i.e., tai_offset will be updated)
int get_num_leapseconds(void)
{
    struct timespec utc, tai, diff;
    int offset;

    clock_gettime(CLOCK_REALTIME, &utc);
    clock_gettime(CLOCK_TAI, &tai);

    time_diff(&utc, &tai, &diff);

    offset = diff.tv_sec;
    if (diff.tv_nsec > 500000000) offset++;

    return offset;
}

int configure_hw_timestamping(int sock_fd)
{
    int flags;
    struct hwtstamp_config hwts_config;
    struct ifreq ifr;


    flags   = SOF_TIMESTAMPING_RX_SOFTWARE
            | SOF_TIMESTAMPING_TX_SOFTWARE
            | SOF_TIMESTAMPING_TX_HARDWARE
            | SOF_TIMESTAMPING_RX_HARDWARE 
            | SOF_TIMESTAMPING_RAW_HARDWARE;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_TIMESTAMPING, &flags, sizeof(flags)) < 0)
    {
        printf("ERROR: setsockopt SO_TIMESTAMPING: [%d]\n", errno);
        return errno;
        }

    /* Enable hardware timestamping on the interface */
    memset(&hwts_config, 0, sizeof(hwts_config));
    hwts_config.tx_type = HWTSTAMP_TX_ON;
    hwts_config.rx_filter = HWTSTAMP_FILTER_ALL;
    memset(&ifr, 0, sizeof(ifr));    
    strncpy(ifr.ifr_name, IF_NAME, sizeof(ifr.ifr_name));
    ifr.ifr_data = (void *)&hwts_config;
    if (ioctl(sock_fd , SIOCSHWTSTAMP, &ifr) == -1)
    {
        printf("failed to set hardware timestamping ioctl");
        return -1;
    }
    if (ioctl(sock_fd , SIOCGHWTSTAMP, &ifr) == -1)
    {
        printf("failed to set hardware timestamping ioctl");
        return -1;
    }
}

/**
 * Should have setup the socket this message came from with 'confgure_hw_timestamping(sock)'
 * And read a message with readmsg(sock). 
 * 
 * The cmsg/control portion of the msg contains the timestamps, and must be allocated with enough space to hold at least 3 timespecs (index 2 is the hardware timestamp)
 */ 
int get_hw_timestamp_from_msg(struct msghdr* msg, struct timespec* ts)
{
    int level, type;
    struct timespec* ts_from_msg;

    int found_timespec = 0;
    struct cmsghdr* cmsg;
    for (cmsg = CMSG_FIRSTHDR(msg); cmsg != NULL; cmsg = CMSG_NXTHDR(msg, cmsg))
    {
        if (SOL_SOCKET == cmsg->cmsg_level && SO_TIMESTAMPING == cmsg->cmsg_type) {
            ts_from_msg = (struct timespec *) CMSG_DATA(cmsg);
            // printf("TIMESTAMP %ld.%09ld\n", (long)ts_from_msg[2].tv_sec, (long)ts_from_msg[2].tv_nsec);
            //the hardware timespec is 
            ts->tv_sec = ts_from_msg[2].tv_sec;
            ts->tv_nsec = ts_from_msg[2].tv_nsec;
            found_timespec = 1;
        }
    }

    return found_timespec;


}

int get_eth_index_num(struct ifreq* ifr)
{
    char* if_name = IF_NAME;
    size_t if_name_len = sizeof(IF_NAME);
    // char* if_name = ETH_INTERFACE_I225_VLAN3;
    // size_t if_name_len = sizeof(ETH_INTERFACE_I225_VLAN3);

    if (if_name_len < sizeof(ifr->ifr_name) ) 
    {
        memcpy(ifr->ifr_name, if_name, if_name_len);
        ifr->ifr_name[if_name_len] = 0;
    } 
    else 
    {
        printf("interface name is too long");
        return -1;
    }

    int fd=socket(AF_UNIX,SOCK_DGRAM,0);
    if (fd==-1) {
        printf("%s",strerror(errno));
        return -abs(errno);
    }

    if (ioctl(fd,SIOCGIFINDEX,ifr)==-1) 
    {
        printf("%s",strerror(errno));
        return -abs(errno);
    }

    return ifr->ifr_ifindex;
}

int get_eth_mac_addr(struct ifreq* ifr)
{
    int rc;
    char* if_name = IF_NAME;
    size_t if_name_len = sizeof(IF_NAME);

    if (if_name_len < sizeof(ifr->ifr_name) ) 
    {
        memcpy(ifr->ifr_name, if_name, if_name_len);
        ifr->ifr_name[if_name_len] = 0;
    } 
    else 
    {
        printf("interface name is too long");
        return -1;
    }

    int fd = socket(AF_UNIX, SOCK_DGRAM, 0);

    rc = ioctl(fd, SIOCGIFHWADDR, ifr);
    if (rc < 0) {
        shutdown(fd, 2);
        return -1;
    }
}

void print_timespec(const struct timespec ts)
{
    printf("T=%ld.%09ld", ts.tv_sec, ts.tv_nsec);
}

/**
 * 
 * source: https://www.guyrutenberg.com/2007/09/22/profiling-code-using-clock_gettime/
 */ 
void time_diff(const struct timespec * older_time, const struct timespec * newer_time, struct timespec* diff)
{
    //  struct timespec diff;
    //  diff.tv_sec = newer_time->tv_sec - older_time->tv_sec;
    //  diff.tv_nsec = newer_time->tv_nsec - older_time->tv_nsec;

    //  if (diff.tv_nsec < 0 && diff.tv_sec == 0)
    //  {
    //       diff.tv_nsec = abs(diff.tv_nsec);
    //  }

    //  while (diff.tv_nsec < 0)
    //  {
    //       diff.tv_nsec += 1000 * 1000 * 1000;
    //       diff.tv_sec--;
    //  }
    //  while (diff.tv_nsec > 1000 * 1000 * 1000)
    //  {
    //       diff.tv_nsec -= 1000 * 1000 * 1000;
    //       diff.tv_sec++;
    //  }

    //  return diff;
    if ((newer_time->tv_nsec - older_time->tv_nsec)<0)
    {
        diff->tv_sec = newer_time->tv_sec - older_time->tv_sec-1;
        diff->tv_nsec = 1000000000 + newer_time->tv_nsec - older_time->tv_nsec;
    }
    else 
    {
        diff->tv_sec = newer_time->tv_sec - older_time->tv_sec;
        diff->tv_nsec = newer_time->tv_nsec - older_time->tv_nsec;
    }
}


int wait(struct timespec sleep_duration, int no_print)
{
    struct timespec remaining_time;
    if (sleep_duration.tv_sec < 0)
    {
        printf("sleep duration is negative; return now.\n");
        return -1;
    }

    if (!no_print) 
    { 
        printf("Wait for "); print_timespec(sleep_duration); printf("\n"); 
    }
    int return_code = nanosleep(&sleep_duration, &remaining_time);
    if (return_code != 0) {
        printf("Nanosleep returned non-zero [%d]; errno: [%d]", return_code, errno);
    }
    return return_code;
}

int wait_until(struct timespec wake_time, int no_print)
{
    struct timespec current_time, sleep_duration;

    clock_gettime(CLOCK_REALTIME, &current_time);

    time_diff(&current_time, &wake_time, &sleep_duration);
    return wait(sleep_duration, no_print);
}


void print_hex(const char* str, int len)
{
    int bytes_left = len;
    printf("%d hex bytes: \n", len)
    printf("\t0x ");
    while (bytes_left-- > 0)
    {
        printf("%d\n", bytes_left)
        printf("%02x  ", (uint8_t) str[len - bytes_left]);
        if ((len-bytes_left) % 20 == 0)
        {
            printf("\n\t0x ");
        }
    }
    
}

/**
 *  int32_t test_id;
    int32_t frame_id;
    int32_t frame_priority;
    struct timespec tx_time;
 */ 
int write_timespec_to_csv(FILE* f, const struct timespec ts, int32_t frame_id, int32_t test_id, int32_t priority)
{
    char str_to_write[256];
    int bytes_written, elements_written;
    bytes_written = snprintf(str_to_write, 256, "%d, %d, %d %ld.%09ld\n", test_id, priority, frame_id, ts.tv_sec, ts.tv_nsec);
    if (bytes_written < 0)
    {
        return -EINVAL;
    }
    elements_written = fwrite(str_to_write, 1, bytes_written, f);
    if (elements_written != bytes_written)
    {
        printf("Error in writing to file: [%d] el, [%d] bytes\n", elements_written, bytes_written);
    }

    return 0;
}
