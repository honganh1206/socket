#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include "utils.h"

/*Because the parent process does NOT always want to block on a call to wait
 * so it lets the signal handler to deal with it.*/
void sigchld_handler(int s) {
    /*Quite unused variable warning
     * but why having it as a param it we are not going to used it?
     * */
    (void)s;
    /*waitpid() might overwrite errno macro when it encounters an error,
     * because it reports failures through errno,
     * so we need to preserve it then restore it as other system calls may alter it */
    int saved_errno = errno;
    
    /*No error from system calls, restore it */
    while (waitpid(-1, NULL, WNOHANG) > 0) {
	errno = saved_errno;
    }
}


int main(void) {
    /*Listen on sockfd, new connection on new_fd */
    int sock_fd, new_fd;
    /*hints contain information about an address of a service provider */
    struct addrinfo hints, *servinfo, *p;
    /*Connector's address info, we use _storage as it is large enough compared to other structs */
    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    /*Action to be taken when signal arrives */
    struct sigaction sa;
    int yes=1;
    /* Store size of Ipv6 address? */
    char s[INET6_ADDRSTRLEN];
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    /* Use my IP*/
    hints.ai_flags = AI_PASSIVE;
    
    /*As server, no need to get hostname */
    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
	fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
	return 1;
    }

    /*Iterate through the returned linked list of nodes containing address info */
    for (p = servinfo; p != NULL; p = p->ai_next) {
	if ((sock_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
	    /* Error fetching socket? */
	    perror("server: socket");
	    continue;
	}

	if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
	    perror("setsockopt");
	    exit(1);
	}

	/*Associate the socket with the local machine */
	if (bind(sock_fd, p->ai_addr, p->ai_addrlen) == -1) {
	    close(sock_fd);
	    perror("server: bind");
	    continue;
	}

	break;
    }

    freeaddrinfo(servinfo);

    if (p == NULL) {
	fprintf(stderr, "server: failed to bind\n");
	exit(1);
    }

    if (listen(sock_fd, BACKLOG) == -1) {
	perror("listen");
	exit(1);
    }

    // sa_handler is a macro, hence no LSP hint?
    // Reap all dead processes
    sa.sa_handler = sigchld_handler;

    /*Clear all signals */
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    /* Get/set the action for signal */
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
	perror("sigaction");
	exit(1);
    }

    printf("server: waiting for connection...\n");

    while(1) {
	sin_size = sizeof their_addr;
	/* Handle the incoming connection and return a brand new socket descriptor
	 * and this new descriptor will handle the following operations */
	new_fd = accept(sock_fd, (struct sockaddr *)&their_addr, &sin_size);
	if (new_fd == -1) {
	    perror("accept");
	    continue;
	}

	/*Convert IP address in binary format to presentation form (string) */
	inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
	printf("server: got connection from %s\n", s);

	if (!fork()) {
	    /*Child process does not need a listener*/
	    close(sock_fd);
	    /*Child process now */
	    if (send(new_fd, "Hello, world!", 13, 0) == -1) {
		perror("send");
	    }
	    close(new_fd);
	    exit(0);
	}
	close(new_fd);
    }

    return 0;
}
