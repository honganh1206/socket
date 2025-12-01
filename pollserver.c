#include <asm-generic/socket.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include "utils.h"


/*A multiperson chat server */
int get_listener_socket(void) {
    int listener; // File descriptor of the listener
    int yes = 1;
    int rv;

    struct addrinfo hints, *ai, *p;

    /*Get a socket and bind it */
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL,CHATSERVERPORT, &hints, &ai)) != 0) {
	fprintf(stderr, "pollserver: %s\n", gai_strerror(rv));
	exit(1);
    }

    for (p = ai; p != NULL; p = p->ai_next) {
	listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
	if (listener < 0) {
	    continue;
	}

	/* Allow this port to be reused immediately after the program stops
	 * We use SOL_SOCKET to set the level to generic socket API instead of protocol-specific*/
	setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

	/* Cannot associate a socket with a port scenario */
	if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
	    close(listener);
	    continue;
	}

	break;
    }

    /*No bound in this case */
    if (p == NULL) {
	return -1;
    }

    freeaddrinfo(ai);

    if (listen(listener, 10) == -1) {
	return -1;
    }

    return listener;
}

/*Add a new file descriptor to the set 
 * pfds works a pointer to a pointer, pointing to the dynamic array of pollfd,
 * with a pointer to the first element of the array a.k.a base address (pointer decay stuff?)
 * */
void add_to_pfds(struct pollfd **pfds, int newfd, int *fd_count, int *fd_size) {
   if (*fd_count == *fd_size) {
       /*Double the size (wow we are generous) */
       *fd_size *= 2;
       *pfds = realloc(*pfds, sizeof(**pfds) * (*fd_size));
   } 

   /*At this point we are manipulating the entire array, not just the pointer */
   (*pfds)[*fd_count].fd = newfd;
   (*pfds)[*fd_count].events = POLLIN; // Check ready-to-read
   (*pfds)[*fd_count].revents = 0; 

   (*fd_count)++;
}

/*Remove a file descriptor at a given index */
void del_from_pfds(struct pollfd pfds[], int i, int *fd_count) {
    /*Copy the last element in the array/set over-top the one you are deleting */
    pfds[i] = pfds[*fd_count - 1]; 
    /*Pass the count as one fewer (this will be passed to poll()) */
    (*fd_count)--;
}

void handle_new_connection(int listener, int *fd_count, int *fd_size, struct pollfd **pfds) {
    /*Client address */
    struct sockaddr_storage remoteaddr;
    socklen_t addrlen;
    /*Newly accepted socket descriptor */
    int newfd;
    /*String representation for accepted IPv6 address */
    char remoteIP[INET6_ADDRSTRLEN];

    addrlen = sizeof remoteaddr;
    /*Open a new socket when a connection arrives
     * This might block when no incoming connections are waiting*/
    newfd = accept(listener, (struct sockaddr *)&remoteaddr, &addrlen);

    if (newfd == -1) {
	perror("accept");
    } else {
	add_to_pfds(pfds, newfd, fd_count, fd_size);

	printf("pollserver: new connection from %s on socket %d\n",
		inet_ntop2(&remoteaddr, remoteIP, sizeof remoteIP),
		newfd);
    }
}

/*Handle regular client's data or client hangups */
void handle_client_data(int listener, int *fd_count, struct pollfd *pfds, int *pfd_i) {
    char buf[256];

    /*Read bytes from file descriptor into buf */
    int nbytes = recv(pfds[*pfd_i].fd, buf, sizeof buf, 0);

    int sender_fd = pfds[*pfd_i].fd;

    if (nbytes <= 0) {
	/*Either error or connection closed */
	if (nbytes == 0) {
	    printf("pollserver: socket %d hung up\n", sender_fd);
	} else {
	    perror("recv");
	}

	/*Close that faulty file/socket descriptor */
	close(pfds[*pfd_i].fd);

	del_from_pfds(pfds, *pfd_i, fd_count);

	(*pfd_i)--;
    } else {
	/*Happy case - We got some data from the client
	 * btw how does this accept the three last arguments?*/
	printf("pollserver: recv from fd %d: %.*s", sender_fd, nbytes, buf);

	/*Send to everyone.
	 * */
	for(int j = 0; j < *fd_count; j++) {
	    int dest_fd = pfds[j].fd;

	    if (dest_fd != listener && dest_fd != sender_fd) {
		if (send(dest_fd, buf, nbytes, 0) == -1) {
		    perror("send");
		}
	    }
	}
    }
}

/*Process all existing connections */
void process_connections(int listener, int *fd_count, int *fd_size, struct pollfd **pfds) {
    for (int i = 0; i < *fd_count; i++) {
	// Check if either POLLIN or POLLHUP is present
	// by first creating a mask representing either POLLIN or POLLHUP with bitwise OR
	// then use bitwise AND to check if bits in the mask are present in revents
	if ((*pfds)[i].revents & (POLLIN | POLLHUP)) {
	    // Got one connection here, but not sure a new listener or regular listener
	    if ((*pfds)[i].fd == listener) {
		// A new connection
		handle_new_connection(listener, fd_count, fd_size, pfds);
	    } else {
		handle_client_data(listener, fd_count, *pfds, &i);
	    }
	}
    }
}

/*Create a listener and connection set/array,
 * then we loop forever processing connections*/
int main(void) {
    int listener;

    int fd_size = 5;
    int fd_count = 0;
    struct pollfd *pfds = malloc(sizeof *pfds * fd_size);

    listener = get_listener_socket();
    if (listener == -1) {
	fprintf(stderr, "error listening to socket\n");
	exit(1);
    }

    pfds[0].fd = listener;
    /*Report ready to read on incoming connection */
    pfds[0].events = POLLIN;

    fd_count = 1;

    /*Write to stdout with break line */
    puts("pollserver: waiting for connections...");

    for(;;) {
	int poll_count = poll(pfds, fd_count, -1);
	if (poll_count == -1) {
	    perror("poll");
	    exit(1);
	}

	process_connections(listener, &fd_count, &fd_size, &pfds);
    }

    free(pfds);
}

