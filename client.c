#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "utils.h"


int main(int argc, char *argv[]) {
    int sock_fd, num_bytes;
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    /*What for? */
    int rv;
    char s[INET6_ADDRSTRLEN];

    if (argc != 2) {
	fprintf(stderr, "usage: client hostname\n");
	exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
	fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
	return 1;
    }

    /* Loop through all the results 
     * and connect to the first one*/
    for (p = servinfo; p != NULL; p = p->ai_next) {
	if ((sock_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
	    /*Socket not exist or has error, skip this one */
	    perror("client: socket");
	    continue;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
	printf("client: attempting connection to %s\n", s);

	/*Open a connection on sock_fd to peer at addr for addrlen bytes long */
	if (connect(sock_fd, p->ai_addr, p->ai_addrlen) == -1) {
	    perror("client: connect");
	    close(sock_fd);
	    continue;
	}

	break;
    }

    if (p == NULL) {
	/*Write formatted output to stream stderr */
  	fprintf(stderr, "client: failed to connect\n");
	return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
    printf("client connected to %s\n", s);

    freeaddrinfo(servinfo);

    /*Why MAXDATASIZE - 1? Maybe because arrays start at 0? */
    if ((num_bytes = recv(sock_fd, buf, MAXDATASIZE - 1, 0)) == -1) {
	perror("recv");
	exit(1);
    }

    // Add null char representing end of string
    buf[num_bytes] = '\0';

    printf("client: received '%s'\n",buf);

    close(sock_fd);

    return 0;
}
