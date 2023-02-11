#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/poll.h>

#include "a1protos.h"
#include "optparser.h"
#include "util.h"

int main(int argc, char *argv[]) {
    struct sockaddr_in servAddr, fromAddr; // Server info from args and  recvTimeResp
    client_arguments args;
    int retval, sockfd; // Poll return value and socket file descriptor

    if (argc != 9) {
        printf("Wrong number of arguments\n");
        exit(-1);
    }

    // parse args and store in struct
    args = client_parseopt(argc, argv);

    // Create UDP socket
    sockfd = createUDPSocket();

    // Initialize struct to store server address, port, and family
    setServer(&servAddr, args.ip_address, args.port);
    memset(&fromAddr, 0, sizeof(fromAddr)); // Zero 

    // Send N TimeRequests
    for (int i = 0; i < args.num_req; i++) {
       sendTimeReq(sockfd, i+1, &servAddr); 
    }

    // TimeResponse storage
    timeResp responses[args.num_req];
    memset(&responses, 0, sizeof(responses));

    // t2 timestamps storage
    struct timespec timeStamps[args.num_req];
    memset(&timeStamps, 0, sizeof(timeStamps));

    // Set pollfd structure to listen for when the socket is ready to read
    struct pollfd fds[1]; // Size = 1 for the single sockfd
    memset(fds, 0 , sizeof(fds));
    fds[0].fd = sockfd;
    fds[0].events = POLLIN;

    for (int i = 0; i < args.num_req; i++) {
        retval = poll(fds, 1, args.timeout * 1000); // Convert args.timeout from sec to millisec

        if (retval < 0) {
            perror("poll() failed\n");
        } else if (retval == 0) {
            break;
        } else {
            timeResp resp = recvTimeResp(sockfd, &fromAddr);
            responses[resp.seq_num - 1] = resp;
            timeStamps[resp.seq_num - 1] = get_realtime();
        }
    }

    for (int i = 0; i < args.num_req; i++) {
        if (responses[i].seq_num != 0) {
            double offset, delay;

            offset = ((responses[i].s_sec - responses[i].c_sec) + (responses[i].s_sec - timeStamps[i].tv_sec)) * .5;
            offset += (((responses[i].s_nano - responses[i].c_nano) + (responses[i].s_nano - timeStamps[i].tv_nsec)) * .5) * (1.0/1000000000);

            delay = timeStamps[i].tv_sec - responses[i].c_sec;
            delay += (timeStamps[i].tv_nsec - responses[i].c_nano) * (1.0/1000000000);
            
            printf("%d: %.4f %.4f\n", responses[i].seq_num, offset, delay);
        } else {
            printf("%d: Dropped\n", i+1);
        }
    }

    exit(EXIT_SUCCESS);
}