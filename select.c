#include <stdio.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

/*File descriptor for standard input */
#define STDIN 0

int main(void) {
    struct timeval tv;
    fd_set readfds;

    tv.tv_sec = 2;
    tv.tv_usec = 500000;

    /*Clear all entries from set */
    FD_ZERO(&readfds);
    FD_SET(STDIN, &readfds);

    /*Example of monitoring readfds set only */
    select(STDIN+1, &readfds, NULL, NULL, &tv);

    if (FD_ISSET(STDIN, &readfds)) {
	printf("A key was pressed!\n");
    } else {
	printf("Timed out.\n");
    }
    return 0;
}

