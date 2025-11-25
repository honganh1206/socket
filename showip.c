#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main(int argc, char *argv[]) {
    struct addrinfo hints, *res, *p;
    int status;
    char ipstr[INET6_ADDRSTRLEN];

    if (argc != 2) {
        fprintf(stderr, "usage: showip <hostname>");
        return 1;
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // Either IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP
    
    /*res got populated with host info (a pointer to a linked list) */
    if ((status = getaddrinfo(argv[1], NULL, &hints, &res)) != 0) {
       fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
       // Other than 1 and 0
       return 2;
    }

    printf("IP addresses for %s:\n\n", argv[1]);

	/*Loop from the first node pointer (also the base address of the linked list)*/
	for(p = res; p != NULL; p = p->ai_next) {
		void *addr;
		char *ipver;
		struct sockaddr_in *ipv4;
		struct sockaddr_in6 *ipv6;

		if (p->ai_family == AF_INET) {
				/*IPv4 */
				/*Casting pointer from generic sockaddr to sockaddr_in */
				ipv4 = (struct sockaddr_in *)p->ai_addr;
				/*Get the internet address */
				addr = &(ipv4->sin_addr);
				ipver = "IPv4";
		} else {
				ipv6 = (struct sockaddr_in6 *)p->ai_addr;
				addr = &(ipv6->sin6_addr);
				ipver = "IPv6";
		}

		/*Convert the IP in string to presentation form (binary) */
		inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
		printf("	%s: %s\n", ipver, ipstr);
	}
		
	freeaddrinfo(res);

	return 0;
}
