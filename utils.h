#ifndef UTILS_H
#define UTILS_H
#define PORT "3490" // the port client will be connecting to

#define MAXDATASIZE 100 // max number of bytes we can get at once
#define BACKLOG 10      // how many pending connections queue will hold

#define MYPORT "4950" // the port users will be connecting to
#define SERVERPORT "4950"
#define MAXBUFLEN 100

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa);
#endif // !DEBUG
