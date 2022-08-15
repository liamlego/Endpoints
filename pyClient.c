#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include "connectors.h"

int pyRequest(char *data, int n, char flag) {
    const char *host = "127.0.0.1";
    const char *port = "5559";

    int r;

    struct addrinfo hints, *resources;

    printf("Configuring pyClient...");

    memset(&hints, 0, sizeof(struct addrinfo));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    r = getaddrinfo(host, port, &hints, &resources);

    if (r != 0) {
        perror("FAILED");
        exit(1);
    }

    int sockfd;
    sockfd = socket(resources->ai_family, resources->ai_socktype, resources->ai_protocol);
    if (sockfd == -1) {
        perror("failed");
        exit(1);
    }

    /* Connect to a server */

    r = connect(sockfd, resources->ai_addr, resources->ai_addrlen);

    if (r==-1) {
        perror("failed to connect");
        exit(1);
    }
    puts("Connected to pyServer");

    int buff_size = 1024;
    char buffer[buff_size];

    r = send(sockfd, &flag, 1, 0);

    if (r == -1) exit(1);

    printf("Receiving Data from Python... ");
    
    r = recv(sockfd, buffer, buff_size, 0);

    if (r > 0) {

        for (int i = 0; i < r && i < n; i++) {
            data[i] = buffer[i];
        }

    }
    puts("Success!");

    close(sockfd);
    printf("Socket closed!\n");

    // Free allocated memory
    freeaddrinfo(resources);

    return r;
}

// int main(int argv, char **argc) {

//     int n = 1024;
//     char data[n];

//     char ch = argc[1][0];

//     int r = getJson(data, n, ch);

//     printf("Data received:\n");
//     for (int i = 0; i < r; i++) {
//         printf("%c", data[i]);
//     }
//     printf("\n");

//     return 0;
// }