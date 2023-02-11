#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "optparser.h"
#include "util.h"
#include "a1protos.h"

#define MAX_CLIENTS 1024

int main(int argc, char *argv[]) {
    struct sockaddr_in servAddr, fromAddr;
    server_arguments args;
    timeReq req;
    int sockfd;

    if (argc < 3 || argc > 5) {
        printf("Wrong number of arguments\n");
        exit(-1);
    }

    // parse args and store in struct
    args = server_parseopt(argc, argv);

    // Create UDP socket
    sockfd = createUDPSocket();

    // Initialize struct to store server address, port, and family
    setIPv4Server(&servAddr, args.port);
    memset(&fromAddr, 0, sizeof(fromAddr));

    // Bind to the local address
    if (bind(sockfd, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0) {
        perror("bind() failed\n");
        exit(-1);
    }

    // Seed the random number generator
    srand(time(NULL));

    // Store state of clients in an array
    client clients[MAX_CLIENTS];
    memset(clients, 0, sizeof(clients));

    // Block until a time request is received
    for (;;) {
        req = recvTimeReq(sockfd, &fromAddr); // Receive Time Request
        struct timespec t = get_realtime(); // Stamp time
        reapStaleClients(clients, MAX_CLIENTS, t); // garbage collect stale states 
        
        // Gen random num btwn [1, 100]. If it is larger than the given percent we do NOT ignore the packet
        if (rand() % 100 + 1 > args.percent) {
            client newClnt = {fromAddr, t, req.seq_num}; // Instantiate new client
            int i = clientIndex(clients, MAX_CLIENTS, newClnt); // Search for the new client in the array

            // Convert binary to printable address
            char address[INET_ADDRSTRLEN]; 
            if (inet_ntop(AF_INET, &fromAddr.sin_addr.s_addr, address, sizeof(address)) == NULL) {
                perror("Invalid address\n");
                exit(-1);
            }

            // If this is the first request from the client, add to state
            if (i == -1) {
                addClient(clients, MAX_CLIENTS, newClnt);

            // The curr seq_num is lower than the saved seq_num, print
            } else if (newClnt.seq_num < clients[i].seq_num) {
                printf("%s:%d %d %d\n", address, ntohs(newClnt.addr.sin_port), newClnt.seq_num, clients[i].seq_num);

            // If the curr seq_num is higher than the saved seq_num, update state 
            } else {
                updateClient(clients, i, newClnt);
                // printf("Updated client %s:%d index: %d seq_num: %d\n", address, ntohs(newClnt.addr.sin_port), i, newClnt.seq_num);
            }

            sendTimeResp(sockfd, req, t, &fromAddr);
        }
    }
    
}