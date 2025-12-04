
#include "utils.h"
#include <asm-generic/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

int get_listener_socket(void) {
    struct addrinfo hints, *ai, *p;
    int yes=1;
    int rv;
    int listener;

    /*Fill value 0 at struct to initialize for later use */
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, CHATSERVERPORT, &hints, &ai)) != 0) {
	fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
	exit(1);
    }

    for (p = ai; p != NULL; p = ai->ai_next) {
	listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
	if (listener < 0) {
	    continue;
	}

	setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

	if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
	    close(listener);
	    continue;
	}

	break;
    }

    if (p == NULL) {
        fprintf(stderr, "selectserver: failed to bind\n");
        exit(2);
    }

    freeaddrinfo(ai);

    if (listen(listener, 10) == -1) {
        perror("listen");
        exit(3);
    }

    return listener;
}

/*Add new incoming connections to sets */
void handle_new_connection(int listener, fd_set *master, int *fdmax) {
    socklen_t addrlen;
    /*Newly accepted socket descriptor */
    int new_fd;
    /*Client address */
    struct sockaddr_storage remoteaddr;
    char remoteIP[INET6_ADDRSTRLEN];

    addrlen = sizeof remoteaddr;

    /*Return a new socket descriptor from the listener */
    new_fd = accept(listener, (struct sockaddr *)&remoteaddr, &addrlen);

    if (new_fd == -1) {
	perror("accept");
    } else {
	/*Add socket of the new connection to the set */
	FD_SET(new_fd, master);
	if (new_fd > *fdmax) {
	    *fdmax = new_fd;
	}
	printf("selectserver: new connection from %s on socket %d\n",
            inet_ntop2(&remoteaddr, remoteIP, sizeof remoteIP),
            new_fd);
    }
}

/*Broadcast a message to all clients */
void broadcast(char *buf, int nbytes, int listener, int s, fd_set *master, int fdmax) {
    for (int j = 0; j <= fdmax; j++) {
	// Part of fd set
	if (FD_ISSET(j, master)) {
	    /*Send to everyone 
	     * except the server's listening socket and the client socket that sends the message*/
	    if (j != listener && j != s) {
		if (send(j, buf, nbytes, 0) == -1) {
		    perror("send");
		}	
	    }
	}
    }
}

void handle_client_data(int s, int listener, fd_set *master, int fdmax) {
    /*Buffer for client data */
    char buf[256];
    int nbytes;

    if ((nbytes = recv(s, buf, sizeof buf, 0)) <= 0) {
	if (nbytes == 0) {
	    printf("selectserver: socket %d hung up\n", s);
	} else {
	    perror("recv");
	}
	close(s);
	/*Remove the socket from the set */
	FD_CLR(s, master);
    } else {
	/*Recursive broadcasting */
	broadcast(buf, nbytes, listener, s, master, fdmax);
    }
}

int main(void) {
    fd_set master;
    /*Temp fd list for select()
     * since select() will modify the set we pass into it (master)*/
    fd_set read_fds;
    int fdmax;

    /*Listening socket descriptor */
    int listener;

    FD_ZERO(&master);
    FD_ZERO(&read_fds);

    listener = get_listener_socket();

    FD_SET(listener, &master);

    /*So far only this one */
    fdmax = listener;

    for (;;) {
	read_fds = master;
	/*No timeout */
	if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
	    perror("select");
            exit(4);
	}

	/*Run through the existing connections */
	for(int i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) { // we got one!!
                if (i == listener)
                    handle_new_connection(i, &master, &fdmax);
                else
                    handle_client_data(i, listener, &master, fdmax);
            }
        }
    }

    return 0;
}
