#include <sys/types.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <unistd.h>

int sendall(int s, char *buf, int *len) {
    int total = 0; // bytes we have sent
    int bytesleft = *len;
    int n;

    while(total < *len) {
	n = send(s, buf+total, bytesleft, 0);
	if (n == -1) { break ; }
	total += n;
	bytesleft -= n;
    }

    *len = total;

    /*-1 on failure, 0 on success */
    return n == -1 ? -1 : 0;
}

int main(void) {
    char buf[] = "Hey!";
    int len = strlen(buf);
    int sock_pair[2];
    char recv_buf[sizeof buf] = {0};

    /*Create two new sockets and put their file descripts in sock_pair */
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sock_pair) == -1) {
        perror("socketpair");
        return 1;
    }

    int sock_fd = sock_pair[0];

    if (sendall(sock_fd, buf, &len) == -1) {
        perror("sendall");
        printf("We only sent %d bytes because of the error!\n", len);
    } else {
        recv(sock_pair[1], recv_buf, sizeof recv_buf, 0);
        printf("Receiver got: %s\n", recv_buf);
    }

    close(sock_pair[0]);
    close(sock_pair[1]);
}
