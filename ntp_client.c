#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "ntp_packet.h"
#define SERVER "pool.ntp.org"
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

int main(int argc, char const *argv[])
{
    // build packet
    ntp_packet packet;
    memset(&packet, 0, sizeof(packet));
    packet.li_vn_mode = 0x23; // 0b00100011
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    packet.txTm_s = spec.tv_sec;
    packet.txTm_f = spec.tv_nsec / 1000;
    printf("%u.%u\n", packet.txTm_s, packet.txTm_f);

    // setup socket
    int socketfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (socketfd < 0)
    {
        perror("error opening socket");
        exit(-1);
    }
    struct hostent *server = gethostbyname(SERVER);
    
    if (server == NULL)
    {
        perror("could not find host");
        printf("%s\n", SERVER);
        exit(-1);
    }
    DEBUG("addr size: %d", server->h_length);
    DEBUG("ip: %x %x %x %x", (unsigned char)server->h_addr_list[0][0], (unsigned char)server->h_addr_list[0][1], (unsigned char)server->h_addr_list[0][2], (unsigned char)server->h_addr_list[0][3]);
    DEBUG("ip: %d.%d.%d.%d", (unsigned char)server->h_addr_list[0][0], (unsigned char)server->h_addr_list[0][1], (unsigned char)server->h_addr_list[0][2], (unsigned char)server->h_addr_list[0][3]);
    struct sockaddr_in serv_addr;
    if (inet_pton(AF_INET, server->h_addr_list[0], &serv_addr.sin_addr.s_addr) != 1)
    {
        perror("could not convert address");
        printf("%.*s\n", server->h_length, server->h_addr_list[0]);
        exit(-1);
    }
    DEBUG("converted %s %s to %d", SERVER, server->h_addr_list[0], serv_addr.sin_addr.s_addr);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    memset(serv_addr.sin_zero, '\0', 8);
    int conn = conn = connect(socketfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
    if (conn < 0)
    {
        perror("connection error");
        exit(-1);
    }
    char *msg = "hey";
    DEBUG("%s sizeof: %lu, strlen: %lu", msg, sizeof(msg), strlen(msg));
    DEBUG("0: %d %c '\\0' %d %c '0' %d %c", 0, 0, '\0', '\0', '0', '0');
    send(socketfd, "hey", strlen(msg), 0);
    char *buffer[BUF_SIZE] = {0};
    int res_size = read(socketfd, buffer, BUF_SIZE);
    printf("response of size %d:\n", res_size);
    for (int i = 0; i < res_size; i++)
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
        printf("%02x ", (char*)buffer[i]);
        
    }
    printf("\nbye!\n");
    return 0;
}