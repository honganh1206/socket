#include <asm-generic/socket.h>
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

#define SERVERPORT 4950    // the port users will be connecting to

/*Send data to multiple hosts at the same time */
int main(int argc, char *argv[]) {
    int sockfd;
    /*Connector's address info */
    struct sockaddr_in their_addr;
    /* Base entry for single host */
    struct hostent *he;
    int numbytes;
    int broadcast = 1;

    if (argc != 3) {
	fprintf(stderr, "usage: broadcaster hostname message\n");
	exit(1);
    }

    if ((he = gethostbyname(argv[1])) == NULL) {
	perror("gethostbyname");
	exit(1);
    }

    /*Datagram? */
    if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) == -1) {
	perror("socket");
	exit(1);
    }

    /*Allow broadcast packets to be sent by setting SO_BROADCAST. Then we can use sendto() to anyone */
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof broadcast) == -1) {
	perror("setsockopt (SO_BROADCAST)");
	exit(1);
    }

    /*host byte order */
    their_addr.sin_family = AF_INET;

    /*network byte order conversion */
    their_addr.sin_port = htons(SERVERPORT);

    /*Get address of host entry? */
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);

    /* Fill address with 0s */
    memset(their_addr.sin_zero, '\0', sizeof their_addr.sin_zero);

    /*Send bytes in buffer from socket to peer at address */
    numbytes = sendto(sockfd, argv[2], strlen(argv[2]), 0, (struct sockaddr *)&their_addr, sizeof their_addr);

    if (numbytes == -1) {
	perror("sendto");
	exit(1);
    }

    /* First convert internet number (presume binary format) to ASCII representation, then print it */
    printf("sent %d bytes to %s\n", numbytes, inet_ntoa(their_addr.sin_addr));

    close(sockfd);

    return 0;
}
