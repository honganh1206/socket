/*Sit on a machine waiting for a packet on port 4950
 * We use IPv6 since datagram sockets are fire-and-forget*/

#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

int main(void) {
    int sock_fd;
    /*servinfo works as the head of the list
     * p works as the cursor to walk through the linked list*/
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int num_bytes;
    /*Info of the address we are about to connect to */
    struct sockaddr_storage their_addr;
    char buf[MAXBUFLEN];
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];

    /* Fill value 0 to block of memory at address hints */
    /*To initialize struct here for later use */
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0) {
	fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
	return 1;
    }

    /*servinfo must be kept unchanged,
     * and if we adnvanced servinfo, we lose the true head
     * thus later on we cannot free it with freeaddrinfo()*/
    for (p = servinfo; p != NULL; p = p->ai_next) {
	if ((sock_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
	    perror("listener: socket");
	    continue;
	}

	/*Associate a socket with a port */
	if (bind(sock_fd, p->ai_addr, p->ai_addrlen) == -1) {
	    close(sock_fd);
	    perror("listener: bind");
	    continue;
	}

	break;
    }

    if (p == NULL) {
	fprintf(stderr, "listener: failed to bind socket\n");
	/*Not an error, just lack resource
	 * so we return 2*/
        return 2;
    }

    /*No need for list head now since we bind the socket successfully */
    freeaddrinfo(servinfo);

    printf("listener: waiting for recvfrom...\n");

    addr_len = sizeof their_addr;

    /*recvfrom() is a BLOCKING system call
     * and it WAITS INDEFINITELY until a datagram arrives*/
    if((num_bytes = recvfrom(sock_fd, buf, MAXBUFLEN - 1, 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) {
	perror("recvfrom");
	exit(1);
    }

    printf("listener: got packet from %s\n", 
	    /*Transform address from binary format to string
	     * and put string into buffer s*/
	    inet_ntop(their_addr.ss_family,
		get_in_addr((struct sockaddr *)&their_addr),
		s, 
		sizeof s));

    printf("listener: packet is %d bytes long\n", num_bytes);
    buf[num_bytes] = '\0';
    printf("listener: packet contains \"%s\"\n", buf);

    close(sock_fd);

    return 0;
}
