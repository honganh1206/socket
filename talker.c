#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

int main(int argc, char *argv[]) {
    int sock_fd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int num_bytes;

    if (argc != 3) {
	fprintf(stderr,"usage: talker hostname message\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_DGRAM;

    /*NOTE: This language design approach for C seems so cool
     * since we only get to return one value here
     * instead of multiple values like in Go,
     * we return only the error if there is one
     * and write the value (if no error) to a memory location */
    rv = getaddrinfo(argv[1], SERVERPORT, &hints, &servinfo);
    if (rv != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
	if ((sock_fd= socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
	    perror("talker: socket");
	    continue;
	}

	break;
    }

    if (p == NULL) {
        fprintf(stderr, "talker: failed to create socket\n"); 
        return 2;
    }

    /*
     * Send len(argv[2]) bytes of buf (argv[2]) at address p->ai_addr.
     * Without a listener, talker will just fire packets into the ether.*/
    if ((num_bytes = sendto(sock_fd, argv[2], strlen(argv[2]), 0, p->ai_addr, p->ai_addrlen)) == -1) {
	perror("talker: sendto");
        exit(1);	
    }

    freeaddrinfo(servinfo);

    printf("talker: sent %d bytes to %s\n", num_bytes, argv[1]);
    close(sock_fd);

    return 0;
}
