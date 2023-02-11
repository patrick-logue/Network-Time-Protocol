#include <stdint.h>

struct timespec get_realtime();

typedef struct timeReq {
    unsigned int seq_num;
    unsigned int ver;
    long c_sec;
    long c_nano;
} timeReq;

void sendTimeReq(int sd, unsigned int seqNum, struct sockaddr_in *servAddr);
timeReq recvTimeReq(int sd, struct sockaddr_in *fromAddr);

typedef struct timeResp {
    unsigned int seq_num;
    unsigned int ver;
    long c_sec;
    long c_nano;
    long s_sec;
    long s_nano;
} timeResp;

void sendTimeResp(int sd, timeReq req, struct timespec t, struct sockaddr_in *clntAddr);
timeResp recvTimeResp(int sd, struct sockaddr_in *fromAddr);

typedef struct client {
	struct sockaddr_in addr;
    struct timespec time_stamp;
	unsigned int seq_num;
} client;

int clientIndex(client clients[], int clntlen, client c);
void addClient(client clients[], int clntlen, client c);
void updateClient(client clients[], int index, client c);
void reapStaleClients(client clients[], int clntlen, struct timespec currtime);