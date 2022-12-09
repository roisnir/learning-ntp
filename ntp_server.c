#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "ntp.h"
#include "utils.h"
#define BUF_SIZE 1024

int main(int argc, char const *argv[]) {
    int server = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (server < 0) {
        ERROR("error opening socket%s", "");
        exit(-1);
    }
    struct sockaddr_in serv_addr, client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(123);
    if (bind(server, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        ERROR("could not bind to port %d", NTP_PORT);
        exit(EXIT_FAILURE);
    }
    socklen_t addr_len;
    int recv_len;
    addr_len = sizeof(client_addr);
    char buffer[BUF_SIZE] = {0};
    printf("READY\r\n");
    while (true) {
        recv_len = recvfrom(server, buffer, BUF_SIZE, MSG_WAITALL, (struct sockaddr *)&client_addr,
                            &addr_len);
        printf("updating NTP...\r\n");
        sendto(server, buffer, recv_len, MSG_CONFIRM, (struct sockaddr *)&client_addr, addr_len);
    }
    shutdown(server, SHUT_RD);
    return 0;
}