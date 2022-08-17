#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>
#include <string.h>

#include "connectors.h"

#define KILOBYTE 1024
#define MEGABYTE 1048576
#define GIGABYTE 1073741824

// Returns 0 for false and 1 for true
int isEqual(char *a, int s1, int e1, char *b, int s2, int e2);
void runSockets(int sockfd, int *keepSockAlive);
int handleConnection(int sockfd, int clientfd);

char *host = "127.0.0.1";
char *port = "7777";

const int num_of_sockets = 1;

const char *disconnectHeader = 
"HTTP/1.1 200 OK\r\n"
"Access-Control-Allow-Origin: *\r\n"
"Connection: close\r\n"
"Content-Type: text/html; charset=utf-8\r\n\r\n"
"closing connection";

const char *jsonHeader = 
"HTTP/1.1 200 OK\r\n"
"Access-Control-Allow-Origin: *\r\n"
"Connection: keep-alive\r\n"
"Keep-Alive: timeout=5\r\n"
"Content-Type: image/base64; charset=utf-8\r\n\r\n"
"";


int main(int argv, char **argc) {

    // Number of sockets
    int sockets[num_of_sockets];

    pthread_t threads[num_of_sockets];

    // for status returns
    int r, serverSock;

    struct addrinfo hints, *server;   

    // Set memory
    memset(&hints, 0, sizeof(struct addrinfo));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    printf("Getting address info... ");

    r = getaddrinfo(host, port, &hints, &server);

    if (r != 0) {
        perror("failed getaddrinfo()");
        exit(1);
    }
    puts("done");

    /* Setting up sockets */

    printf("Setting up server socket... ");

    serverSock = socket(server->ai_family, server->ai_socktype, server->ai_protocol);
    if (serverSock == -1) {
        perror("failed to load socket");
        exit(1);
    }

    puts("done");
    
    /* Binding sockets */

    printf("Binding socket... \n");

    r = bind(serverSock, server->ai_addr, server->ai_addrlen);

    if (r == -1) {
        perror("failed to bind a socket");
        exit(1);
    }

    puts("done");

    /* Initializing sockets to listen */

    printf("Listening on port 7777... ");

    r = listen(serverSock, 3);

    if (r == -1) {
        perror("a socket failed to listen");
        exit(1);
    }

    puts("");

    /* Start Client Accepting sockets */

    int alive = 1;
    int pid;

    //pid = fork();

    //if (pid != 0) 
    runSockets(serverSock, &alive);

    //if (pid == 0) {

        // char input;

        // do {
        //     scanf("%c", &input);
        // } while (input != 'q');

    printf("Shutting down server... ");

    free(server);
    close(serverSock);
    puts("done");

    return 0;
    //}
}

// Returns 0 for false and 1 for true
int isEqual(char *a, int s1, int e1, char *b, int s2, int e2) {

    if (e1-s1 != e2-s2) return 0;

    while (s1 <= e1 && s2 <= e2) {
        if (a[s1] != b[s2]) return 0;
        s1++;
        s2++;
    }
    
    return 1;
}

// sockfd = socket file descripter
// Starts accepting requests
void runSockets(int sockfd, int *keepSockAlive) {
    
    //This method propogates too many
    // socket processes if a user keeps reconnecting
    int parentPid;
    int pid = 1;
    while (*keepSockAlive == 1) {

        struct sockaddr client_address;
        socklen_t client_len;

        int clientfd;

        client_len = sizeof(client_address);

        printf("Accepting... ");

        clientfd = accept(sockfd, &client_address, &client_len);

        if (clientfd == -1) {
            perror("failed client connection");
            exit(1);
        }
        puts("Established connection with client\n");

        parentPid = getpid();

        int isChild;
        pid = fork();
        isChild = pid;

        pid = getpid();
        
        printf("%d == %d\n", parentPid, pid);
        if (parentPid == pid) {
            // Will halt loop here for duration of connection
            handleConnection(sockfd, clientfd);
        } 
        // essentially need to close twice because when we
        // fork, it duplicates
        close(clientfd);

        if (isChild && parentPid == pid) {
            printf("Die process!\n");
            break;
        } else {
            printf("Another!\n");
        }
    }
}


int handleConnection(int sockfd, int clientfd) {

    int n = 1;

    int buffer_length = 1024;

    char buffer[buffer_length];

    printf("Receiving data... ");

    // recv returns number of bytes received
    // and stores it in r
    int r = recv(clientfd, buffer, buffer_length, 0);

    if (r > 0) {
        printf("Receiving %d bytes\n", r);

        int equality = isEqual("/endpoint/7", 0, 10, buffer, 4, 14);

        if (equality == 0) {
            printf("\nMISSED ENDPOINT\n");

            r = send(clientfd, disconnectHeader, strlen(disconnectHeader), 0);
            
            if (r == -1) {
                perror("Failed to send data");
                exit(1);
            }
            puts("SENT CLOSE HEADER");

            
        } else {
            printf("\nHIT ENDPOINT\n");

            // Sending HEADER
            r = send(clientfd, jsonHeader, strlen(jsonHeader), 0);

            if (r == -1) exit(1);

            int n = 27*KILOBYTE;
            char sendbuff[n];

            FILE *image;
            image = fopen("mhm.PNG", "rb");

            // fseek(image, 0L, SEEK_END);
            // int sz = ftell(image);
            // rewind(image);

            r = fread(sendbuff, n, 1, image);

            char message[n];
            // Request base64 encoding data from pyServer
            r = pyRequest(message, n, sendbuff, n);

            printf("Received %d bytes from pyRequest\n", r);

            // Sending base64 image to client
            r = send(clientfd, message, r, 0);

            if (r == -1) exit(1);

            printf("Sent %d bytes\n", r);

            if (r == -1) {
                perror("Failed to send data");
                exit(1);
            }
            puts("SENT HEADER");

        }
    }
}