#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>
#include "utils.h"

void *get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

const char *inet_ntop2(void *addr, char *buf, size_t size) {
    struct sockaddr_storage *sas = addr;
    struct sockaddr_in *sa4;
    struct sockaddr_in6 *sa6;
    void *src;

    switch (sas->ss_family) {
	case AF_INET:
	    /* Assign generic pointer type to concrete type? How? */
	    sa4 = addr;
	    src = &(sa4->sin_addr);
	    break;
	case AF_INET6:
	    sa6 = addr;
	    src = &(sa6->sin6_addr);
	    break;
	default:
	    return NULL;
    }

    return inet_ntop(sas->ss_family, src, buf, size);
}

