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
#include "utils.h"
#define DEFAULT_SERVER "ntp.ubuntu.com"
// #define SERVER "pool.ntp.org"
#define PORT 123
#define BUF_SIZE 1024
#define NTP_MODE(b) (b & 0x7)
#define NTP_VERSION(b) ((b >> 3) & 0x7)
#define NTP_LEAP_INDICATOR(b) (b >> 6)
#define NTP_TIMESTAMP_DELTA 2208988800ul

void ntoh_ntp_packet(ntp_packet *packet){
    packet->rootDelay_s = ntohs(packet->rootDelay_s);
    packet->rootDelay_f = ntohs(packet->rootDelay_f);
    packet->rootDispersion_s = ntohs(packet->rootDispersion_s);
    packet->rootDispersion_f = ntohs(packet->rootDispersion_f);
    packet->refTm_s = ntohl(packet->refTm_s);
    packet->refTm_f = ntohl(packet->refTm_f);
    packet->origTm_s = ntohl(packet->origTm_s);
    packet->origTm_f = ntohl(packet->origTm_f);
    packet->rxTm_s = ntohl(packet->rxTm_s);
    packet->rxTm_f = ntohl(packet->rxTm_f);
    packet->txTm_s = ntohl(packet->txTm_s);
    packet->txTm_f = ntohl(packet->txTm_f);
}

void hton_ntp_packet(ntp_packet *packet){
    packet->rootDelay_s = htons(packet->rootDelay_s);
    packet->rootDelay_f = htons(packet->rootDelay_f);
    packet->rootDispersion_s = htons(packet->rootDispersion_s);
    packet->rootDispersion_f = htons(packet->rootDispersion_f);
    packet->refTm_s = htonl(packet->refTm_s);
    packet->refTm_f = htonl(packet->refTm_f);
    packet->origTm_s = htonl(packet->origTm_s);
    packet->origTm_f = htonl(packet->origTm_f);
    packet->rxTm_s = htonl(packet->rxTm_s);
    packet->rxTm_f = htonl(packet->rxTm_f);
    packet->txTm_s = htonl(packet->txTm_s);
    packet->txTm_f = htonl(packet->txTm_f);
}

uint32_t ntp_ts_to_epoch(uint32_t ntp_ts){
    if (ntp_ts == 0){
        return 0;
    }
    return ntp_ts - NTP_TIMESTAMP_DELTA;
}

void print_ntp_packet(ntp_packet *pkt)
{
    char refts[35];
    char orgts[35];
    char rects[35];
    char trats[35];
    isoformatts(refts, ntp_ts_to_epoch(pkt->refTm_s) , pkt->refTm_f);
    isoformatts(orgts, ntp_ts_to_epoch(pkt->origTm_s), pkt->origTm_f);
    isoformatts(rects, ntp_ts_to_epoch(pkt->rxTm_s), pkt->rxTm_f);
    isoformatts(trats, ntp_ts_to_epoch(pkt->txTm_s), pkt->txTm_f);
    char* ref_id;
    if (pkt->stratum == 1){
        ref_id = (char *)&pkt->refId;
    } else {
        struct in_addr tmp;
        tmp.s_addr = pkt->refId;
        ref_id = inet_ntoa(tmp);
    }
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
           pkt->rootDelay_s + (((double)pkt->rootDelay_f) / 0xFFFF),
           pkt->rootDispersion_s + ((((double)pkt->rootDispersion_f)) / 0xFFFF),
           ref_id,
           refts,
           orgts,
           rects,
           trats);
}

int query_ntp_server(ntp_packet *res, in_addr_t server_ip)
{
    // build packet
    ntp_packet packet;
    memset(&packet, 0, sizeof(packet));
    packet.li_vn_mode = 0x23; // 0b00100011
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    packet.txTm_s = (u_int32_t)(spec.tv_sec + NTP_TIMESTAMP_DELTA);
    packet.txTm_f = spec.tv_nsec / 1000;
    // setup socket
    int socketfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (socketfd < 0)
    {
        perror("error opening socket");
        exit(-1);
    }
    struct timeval read_timeout;
    read_timeout.tv_sec = 10;
    read_timeout.tv_usec = 0;
    setsockopt(socketfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&read_timeout, sizeof read_timeout);
    struct sockaddr_in serv_addr;
    serv_addr.sin_addr.s_addr = server_ip;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    memset(serv_addr.sin_zero, '\0', 8);
    int conn = connect(socketfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
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
    hton_ntp_packet(&packet);
    send(socketfd, &packet, sizeof(packet), 0);
    memset(res, 0, sizeof(ntp_packet));
    int res_size = recv(socketfd, (char *)res, sizeof(ntp_packet), 0);
    printf("got response of size %d:\n", res_size);
    if (res_size != sizeof(ntp_packet))
    {
        perror("bad ntp server response");
        printf("response of size %d from %s\n", res_size, inet_ntoa(serv_addr.sin_addr));
        close(socketfd);
        return -1;
    }
    close(socketfd);
    ntoh_ntp_packet(res);
    return 0;
}

int main(int argc, char const *argv[])
{
    in_addr_t server_ip = 0;
    char *server_url = NULL;
    if (argc == 1)
    {
        server_url = DEFAULT_SERVER;
    }
    else if (argc == 3)
    {
        if (strcmp(argv[1], "--ip") == 0)
        {
            server_ip = inet_network(argv[2]);
        }
        else if (strcmp(argv[1], "--url") == 0)
        {
            server_url = argv[3];
        }
        else
        {
            ERROR("Invalid arguments%s", "");
            exit(-1);
        }
    }
    else
    {
        ERROR("Invalid arguments%s", "");
        exit(-1);
    }
    if (server_ip == 0)
    {
        struct hostent *server = gethostbyname(server_url);
        if (server == NULL)
        {
            perror("could not find host");
            printf("%s\n", server_url);
            exit(-1);
        }
        DEBUG("addr size: %d", server->h_length);
        DEBUG("ip: %d.%d.%d.%d", (unsigned char)server->h_addr_list[0][0], (unsigned char)server->h_addr_list[0][1], (unsigned char)server->h_addr_list[0][2], (unsigned char)server->h_addr_list[0][3]);
        DEBUG("converted %s to %u: %s", server_url, *((in_addr_t *)server->h_addr_list[0]), inet_ntoa(*(struct in_addr *)server->h_addr_list[0]));
        server_ip = *((in_addr_t *)server->h_addr_list[0]);
    }
    ntp_packet res;
    int rc = 0;
    rc = query_ntp_server(&res, server_ip);
    if (rc != -1)
    {
        printf("received ntp response: <-- <-- <-- <-- <-- <-- <-- <-- <--\n");
        print_hex_buffer((unsigned char *)&res, 48);
        print_ntp_packet(&res);
        printf("----------------------------------------\n");
    }
    while (rc != -1 && res.stratum > 1)
    {
        rc = query_ntp_server(&res, res.refId);
        if (rc != -1)
        {
            printf("received ntp response: <-- <-- <-- <-- <-- <-- <-- <-- <--");
            print_hex_buffer((unsigned char *)&res, 48);
            print_ntp_packet(&res);
            printf("----------------------------------------\n");
        }
    }
    return 0;
}