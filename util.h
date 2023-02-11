int createUDPSocket();
void setServer(struct sockaddr_in *servAddr, char *ip, int port);
void setIPv4Server(struct sockaddr_in *servAddr, int port);