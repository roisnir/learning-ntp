#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <math.h>
#include "ntp_packet.h"
#define SERVER "ntp.ubuntu.com"
// #define SERVER "pool.ntp.org"
#define PORT 123
#define BUF_SIZE 1024
#define DEBUG_FLAG 1
#define DEBUG(fmt, ...)                                              \
    do                                                               \
    {                                                                \
        if (DEBUG_FLAG)                                              \
            fprintf(stderr, "DEBUG:%s:%d:%s(): " fmt "\n", __FILE__, \
                    __LINE__, __func__, __VA_ARGS__);                \
    } while (0)
#define NTP_MODE(b) (b & 0x7)
#define NTP_VERSION(b) ((b >> 3) & 0x7)
#define NTP_LEAP_INDICATOR(b) (b >> 6)
#define NTP_TIMESTAMP_DELTA 2208988800ul

void print_hex_buffer(unsigned char* buf, int count){
    for (int i = 0; i < count; i++)
    {
        if (i > 0 && i % 8 == 0)
        {
            if (i % 16 == 0)
            {
                printf("\n");
            }
            else
            {
                printf(" ");
            }
        }
        printf("%02x ", (unsigned char)buf[i]);
        
    }
    printf("\n");
}

void strftime_ts(char dst[35], uint32_t sec, uint32_t usec){
    if (sec == 0){
        sprintf(dst, "NULL");
        return;
    }
    char tmp[22] = {0};
    time_t tmpts = sec - NTP_TIMESTAMP_DELTA;
    struct tm *tmptime = localtime(&tmpts);
    strftime(tmp, 21, "%FT%H:%M", tmptime);
    sprintf(dst, "%s:%f", tmp, tmptime->tm_sec + ((double)usec / 0xFFFFFFFF));
}

void print_ntp_packet(ntp_packet* pkt){
    char refts[35];
    strftime_ts(refts, ntohl(pkt->refTm_s), ntohl(pkt->refTm_f));
    char orgts[35];
    strftime_ts(orgts, ntohl(pkt->origTm_s), ntohl(pkt->origTm_f));
    char rects[35];
    strftime_ts(rects, ntohl(pkt->rxTm_s), ntohl(pkt->rxTm_f));
    char trats[35];
    strftime_ts(trats, ntohl(pkt->txTm_s), ntohl(pkt->txTm_f));
    struct in_addr tmp;
    tmp.s_addr = pkt->refId;
    printf("NTP packet:\n"
           "  Flags:\n"
           "    Leap Indicator: %d\n"
           "    Version: %d\n"
           "    Mode: %d\n"
           "  Peer Clock Stratum: %d\n"
           "  Peer Polling Interval: %f\n"
           "  Peer Clock Precision: %f\n"
           "  Root Delay: %f\n"
           "  Root Dispersion: %f\n"
           "  Reference ID: %s\n"
           "  Reference Timestamp: %s\n"
           "  Origin Timestamp: %s\n"
           "  Receive Timestamp: %s\n"
           "  Transmit Timestamp: %s\n",
           NTP_LEAP_INDICATOR(pkt->li_vn_mode),
           NTP_VERSION(pkt->li_vn_mode),
           NTP_MODE(pkt->li_vn_mode),
           pkt->stratum,
           pow(2, (double)pkt->poll),
           pow(2, (double)pkt->precision),
           ntohs(pkt->rootDelay_s) + (((double)ntohs(pkt->rootDelay_f))/0xFFFF),
           ntohs(pkt->rootDispersion_s) + ((((double)ntohs(pkt->rootDispersion_f))) / 0xFFFF),
           inet_ntoa(tmp),
           refts,
           orgts,
           rects,
           trats
           );
}


int query_ntp_server(ntp_packet* res, in_addr_t server_ip){
    // build packet
    ntp_packet packet;
    memset(&packet, 0, sizeof(packet));
    packet.li_vn_mode = 0x23; // 0b00100011
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    packet.txTm_s = htonl(((u_int32_t)spec.tv_sec) + NTP_TIMESTAMP_DELTA);
    packet.txTm_f = htonl(spec.tv_nsec / 1000);
    printf("%u.%u\n", packet.txTm_s, packet.txTm_f);

    // setup socket
    int socketfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    perror("DEBUGGGGGGGG");
    if (socketfd < 0)
    {
        perror("error opening socket");
        exit(-1);
    }
    // struct timeval read_timeout;
    // read_timeout.tv_sec = 5;
    // read_timeout.tv_usec = 0;
    // setsockopt(socketfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&read_timeout, sizeof read_timeout);
    struct sockaddr_in serv_addr;
    serv_addr.sin_addr.s_addr = server_ip;    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    memset(serv_addr.sin_zero, '\0', 8);
    int conn = connect(socketfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
    perror("DEBUGGGGGGGG");
    if (conn < 0)
    {
        perror("connection error");
        exit(-1);
    }
    DEBUG("sending %ld bytes", sizeof(packet));
    printf("sending ntp packet: --> --> --> --> --> --> --> --> --> --> --> --> --> \n");
    print_hex_buffer((unsigned char *)&packet, 48);
    print_ntp_packet(&packet);
    printf("----------------------------------------\n");
    send(socketfd, &packet, sizeof(packet), 0);
    perror("DEBUGGGGGGGG");
    memset(res, 0, sizeof(ntp_packet));
    int res_size = recv(socketfd, (char *)res, sizeof(ntp_packet), 0);
    printf("got response of size %d:\n", res_size);
    if (res_size != sizeof(ntp_packet)){
        perror("bad ntp server response");
        printf("response of size %d from %s\n", res_size, inet_ntoa(serv_addr.sin_addr));
        close(socketfd);
        return -1;
    }
    close(socketfd);
    return 0;
}


int main(int argc, char const *argv[])
{
    struct hostent *server = gethostbyname(SERVER);
    if (server == NULL)
    {
        perror("could not find host");
        printf("%s\n", SERVER);
        exit(-1);
    }
    DEBUG("addr size: %d", server->h_length);
    DEBUG("ip: %d.%d.%d.%d", (unsigned char)server->h_addr_list[0][0], (unsigned char)server->h_addr_list[0][1], (unsigned char)server->h_addr_list[0][2], (unsigned char)server->h_addr_list[0][3]);
    DEBUG("converted %s to %u: %s", SERVER, *((in_addr_t *)server->h_addr_list[0]), inet_ntoa(*(struct in_addr *)server->h_addr_list[0]));
    ntp_packet res;
    int rc = 0;
    rc = query_ntp_server(&res, *((in_addr_t *)server->h_addr_list[0]));
    if (rc != -1){
        printf("recived ntp response: <-- <-- <-- <-- <-- <-- <-- <-- <--");
        print_hex_buffer((unsigned char *)&res, 48);
        print_ntp_packet(&res);
        printf("----------------------------------------\n");
    }
    while (rc != -1 && res.stratum > 1){
        rc = query_ntp_server(&res, res.refId);
        if (rc != -1){
            printf("recived ntp response: <-- <-- <-- <-- <-- <-- <-- <-- <--");
            print_hex_buffer((unsigned char *)&res, 48);
            print_ntp_packet(&res);
            printf("----------------------------------------\n");
        }
    }
    return 0;
}