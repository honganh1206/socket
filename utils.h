#include <stddef.h>

#ifndef UTILS_H
#define UTILS_H
#define PORT "3490" // the port client will be connecting to

#define MAXDATASIZE 100 // max number of bytes we can get at once
#define BACKLOG 10      // how many pending connections queue will hold

#define MYPORT "4950" // the port users will be connecting to
#define SERVERPORT "4950"
#define MAXBUFLEN 100

#define CHATSERVERPORT "9034" // port for multiperson chat server
// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa);

/*Convert socket to IP address string without the hairy detail of switching
 * between IPv4 and IPv6. It returns const char * because inet_ntop does it.*/
const char *inet_ntop2(void *addr, char *buf, size_t size);
#endif // !DEBUG
