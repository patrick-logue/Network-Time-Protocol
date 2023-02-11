#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int createUDPSocket() {
    // Create UDP socket
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket() failed\n");
        exit(-1);
    }
    return sockfd;
}

void setServer(struct sockaddr_in *servAddr, char *ip, int port) {
    memset(servAddr, 0, sizeof(*servAddr)); // Zero
    servAddr->sin_family = AF_INET; // Set family to IPv4
    servAddr->sin_port = htons(port);
    
    int rtnValue = inet_pton(AF_INET, ip, &servAddr->sin_addr.s_addr);
    if (rtnValue == 0) {
        perror("inet_pton() failed, invalid IPv4 address string\n");
        exit(-1);
    } else if (rtnValue < 0) {
        perror("inet_pton() failed\n");
        exit(-1);
    }
}

void setIPv4Server(struct sockaddr_in *servAddr, int port) {
    memset(servAddr, 0, sizeof(*servAddr)); // Zero
    servAddr->sin_family = AF_INET; // Set family to IPv4
    servAddr->sin_port = htons(port); // Ensure binary value is formatted as required by API
    servAddr->sin_addr.s_addr = htonl(INADDR_ANY); // Accept any IPv3 address
}