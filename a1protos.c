#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <time.h>
#include <endian.h>
#include <arpa/inet.h>

#include "a1protos.h"

#define MAX_CLIENTS 512

struct timespec get_realtime() {
    struct timespec t;

    if (clock_gettime(CLOCK_REALTIME, &t) == -1) {
        perror("clock_gettime() failed\n");
        exit(-1);
    }

    return t;
}

void sendTimeReq(int sd, unsigned int seqNum, struct sockaddr_in *servAddr) {
    struct timespec t;
    timeReq req;
    memset(&req, 0, sizeof(req));

    // Stamp current time
    t = get_realtime();

    // Prep TimedRequest
    req.seq_num = htonl(seqNum);
    req.ver = htonl(7);
    req.c_sec = htobe64(t.tv_sec); // Convert from host byte order to big-endian order
    req.c_nano = htobe64(t.tv_nsec);

    // Send TimedRequest
    ssize_t numBytes = sendto(sd, &req, sizeof(req), 0, (struct sockaddr *) servAddr, sizeof(*servAddr));
    if (numBytes < 0) {
        perror("sendto() failed\n");
        exit(-1);
    } else if (numBytes != sizeof(req)) {
        perror("sendto() error, send wrong num of bytes\n");
        exit(-1);
    }
}

void sendTimeResp(int sd, timeReq req, struct timespec t, struct sockaddr_in *clntAddr) {
    timeResp resp;
    memset(&resp, 0, sizeof(resp));

    resp.seq_num = htonl(req.seq_num);
    resp.ver = htonl(req.ver);
    resp.c_sec = htobe64(req.c_sec);
    resp.c_nano = htobe64(req.c_nano);
    resp.s_sec = htobe64(t.tv_sec);
    resp.s_nano = htobe64(t.tv_nsec);

    ssize_t numBytes = sendto(sd, &resp, sizeof(resp), 0, (struct sockaddr *) clntAddr, sizeof(*clntAddr));
    if (numBytes < 0) {
        perror("sendto() failed\n");
        exit(-1);
    } else if (numBytes != sizeof(resp)) {
        perror("sendto() error, send wrong num of bytes\n");
        exit(-1);
    }
}

timeResp recvTimeResp(int sd, struct sockaddr_in *fromAddr) { 
    timeResp resp;
    socklen_t size = sizeof(*fromAddr);

    ssize_t numBytes = recvfrom(sd, &resp, sizeof(resp), 0, (struct sockaddr *) fromAddr, &size);
    if (numBytes < 0) {
        perror("recvfrom() failed\n");
        exit(-1);
    } else if (numBytes != sizeof(resp)) {
        perror("recvfrom() error, received wrong num of bytes\n");
        exit(-1);
    }

    resp.seq_num = ntohl(resp.seq_num);
    resp.ver = ntohl(resp.ver);
    resp.c_sec = be64toh(resp.c_sec); // Network Byte order (big-endian) to host byte order
    resp.c_nano = be64toh(resp.c_nano);
    resp.s_sec = be64toh(resp.s_sec);
    resp.s_nano = be64toh(resp.s_nano);

    return resp;
}

timeReq recvTimeReq(int sd, struct sockaddr_in *fromAddr) {
    timeReq req;
    socklen_t size = sizeof(*fromAddr);

    // Receive 
    ssize_t numBytes = recvfrom(sd, &req, sizeof(req), 0, (struct sockaddr *) fromAddr, &size);
    if (numBytes < 0) {
        perror("recvfrom() failed\n");
        exit(-1);
    } else if (numBytes != sizeof(req)) {
        perror("recvfrom() error, received wrong num of bytes\n");
        exit(-1);
    }

    req.seq_num = ntohl(req.seq_num);
    req.ver = ntohl(req.ver);
    req.c_sec = be64toh(req.c_sec);
    req.c_nano = be64toh(req.c_nano);

    return req;
}

int clientIndex(client clients[], int clntlen, client c) {
    for (int i = 0; i < clntlen; i++) {
        if (clients[i].addr.sin_addr.s_addr == c.addr.sin_addr.s_addr 
            && (clients[i].addr.sin_port == c.addr.sin_port)) {
            return i;
        }
    }
    return -1;
}

void addClient(client clients[], int clntlen, client c) {
    for (int i = 0; i < clntlen; i++) {
        if (clients[i].seq_num == 0) {
            clients[i] = c;
            break;
        }
    }
}

void updateClient(client clients[], int index, client c) {
    clients[index] = c;
}

void reapStaleClients(client clients[], int clntlen, struct timespec currtime) {
    for (int i = 0; i < clntlen; i++) {
        if (clients[i].seq_num != 0 && currtime.tv_sec - clients[i].time_stamp.tv_sec > 120) {
            memset(clients + i, 0, sizeof(client));
        }
    }
}