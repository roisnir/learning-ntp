/*
Simple ntp client, for querying ntp server and finding the stratum 1 server.
This client doesn't affect the system's time.
*/
#include <arpa/inet.h>
#include <math.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "ntp.h"
#include "utils.h"
#define DEFAULT_SERVER "ntp.ubuntu.com"  // pool.ntp.org works also
#define BUF_SIZE 1024
#define RESPONSE_TIMEOUT_SEC 10
#define RESPONSE_TIMEOUT_USEC 0


/*
Sends an ntp client packet with the current time to `server_ip` and sets the
respose to `res`.
The request has a timeout of `RESPONSE_TIMEOUT_SEC` + `RESPONSE_TIMEOUT_USEC`.
The function also prints the request and response.
res: pointer to which the server's response will be set
server_ip: ip address of an ntp server
returns: 0 on success. else, negative integer.
*/
int query_ntp_server(ntp_packet *res, in_addr_t server_ip) {
    // build packet
    ntp_packet packet;
    memset(&packet, 0, sizeof(packet));
    packet.li_vn_mode = 0x23;  // 0b00100011
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    packet.txTm_s = (u_int32_t)(spec.tv_sec + NTP_TIMESTAMP_DELTA);
    packet.txTm_f = spec.tv_nsec / 1000;
    // setup socket
    int socketfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (socketfd < 0) {
        ERROR("failed opening socket%s", "");
        return -1;
    }
    struct timeval read_timeout;
    read_timeout.tv_sec = RESPONSE_TIMEOUT_SEC;
    read_timeout.tv_usec = RESPONSE_TIMEOUT_USEC;
    setsockopt(socketfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&read_timeout,
               sizeof read_timeout);
    struct sockaddr_in serv_addr;
    serv_addr.sin_addr.s_addr = server_ip;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    memset(serv_addr.sin_zero, '\0', 8);
    int conn =
        connect(socketfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (conn < 0) {
        ERROR("failed connecting to server%s", "");
        return -1;
    }
    DEBUG("sending %ld bytes", sizeof(packet));
    printf(
        "sending ntp packet: --> --> --> --> --> --> --> --> --> --> --> --> "
        "--> \n");
    print_hex_buffer((unsigned char *)&packet, 48);
    print_ntp_packet(&packet);
    printf("----------------------------------------\n");
    hton_ntp_packet(&packet);
    send(socketfd, &packet, sizeof(packet), 0);
    memset(res, 0, sizeof(ntp_packet));
    int res_size = recv(socketfd, (char *)res, sizeof(ntp_packet), 0);
    printf("got response of size %d:\n", res_size);
    if (res_size != sizeof(ntp_packet)) {
        ERROR("bad ntp server response (size %d)", res_size);
        close(socketfd);
        return -1;
    }
    close(socketfd);
    ntoh_ntp_packet(res);
    return 0;
}


/*
Queries ntp servers to reach for the stratum 1 server.
*/
int main(int argc, char const *argv[]) {
    in_addr_t server_ip = 0;
    const char *server_url = NULL;
    if (argc == 1) {
        server_url = DEFAULT_SERVER;
    } else if (argc == 3) {
        if (strcmp(argv[1], "--ip") == 0) {
            server_ip = inet_network(argv[2]);
        } else if (strcmp(argv[1], "--host") == 0) {
            server_url = argv[3];
        } else {
            ERROR("Invalid arguments%s", "");
            exit(-1);
        }
    } else {
        ERROR("Invalid arguments%s", "");
        exit(-1);
    }
    if (server_ip == 0) {
        struct hostent *server = gethostbyname(server_url);
        if (server == NULL) {
            perror("could not find host");
            printf("%s\n", server_url);
            exit(-1);
        }
        DEBUG("addr size: %d", server->h_length);
        DEBUG("ip: %d.%d.%d.%d", (unsigned char)server->h_addr_list[0][0],
              (unsigned char)server->h_addr_list[0][1],
              (unsigned char)server->h_addr_list[0][2],
              (unsigned char)server->h_addr_list[0][3]);
        DEBUG("converted %s to %u: %s", server_url,
              *((in_addr_t *)server->h_addr_list[0]),
              inet_ntoa(*(struct in_addr *)server->h_addr_list[0]));
        server_ip = *((in_addr_t *)server->h_addr_list[0]);
    }
    ntp_packet res;
    int rc = 0;
    rc = query_ntp_server(&res, server_ip);
    if (rc != -1) {
        printf("received ntp response: <-- <-- <-- <-- <-- <-- <-- <-- <--\n");
        print_hex_buffer((unsigned char *)&res, 48);
        print_ntp_packet(&res);
        printf("----------------------------------------\n");
    } else {
        ERROR("ntp query failed%s", "");
        exit(-1);
    }
    while (rc != -1 && res.stratum > 1) {
        rc = query_ntp_server(&res, res.refId);
        if (rc != -1) {
            printf(
                "received ntp response: <-- <-- <-- <-- <-- <-- <-- <-- <--");
            print_hex_buffer((unsigned char *)&res, 48);
            print_ntp_packet(&res);
            printf("----------------------------------------\n");
        }else {
        ERROR("ntp query failed%s", "");
        exit(-1);
    }
    }
    return 0;
}