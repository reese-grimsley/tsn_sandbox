#include "helpers.h"


int configure_hw_timestamping(int sock_fd)
{
    int flags;
    struct hwtstamp_config hwts_config;
    struct ifreq ifr;


    flags   = SOF_TIMESTAMPING_RX_SOFTWARE
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
    strncpy(ifr.ifr_name, ETH_INTERFACE_I225, sizeof(ifr.ifr_name));
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
    for (cmsg = CMSG_FIRSTHDR(&msg); cmsg != NULL; cmsg = CMSG_NXTHDR(&msg, cmsg))
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