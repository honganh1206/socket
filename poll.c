#include <stdio.h>
#include <poll.h>
#include <sys/poll.h>

/*Wait for 2.5 seconds for data to be ready to be read from standard input (when we hit RETURN aka Enter) */
int main(void) {
    /*We can monitor more */
    struct pollfd pfds[1];

    /*Why standard input? */
   pfds[0].fd = 0;

   /*Tell when data is ready to recv() on this socket */
   pfds[0].events = POLLIN;

   printf("Hit RETURN or wait 2.5 seconds for timeout\n");

   /*
    * poll() returns the number of elements in the pfds array that have events occurred
    * and those elements have non-zero revents field.
    * We can use realloc() to have enough space for the array.
    * And if we want to delete: Either copy the last element in the array to the index of the one we want to delete, then delete the last element (only work for unordered array) 
    * or set the fd field on the negative number for poll() to ignore it
    **/
   int num_events = poll(pfds, 1, 2500);

   if (num_events == 0) {
       printf("Poll timed out!\n");
   } else {
       /*Bitwise OR between events on return and data ready to be received? */
       int pollin_happened = pfds[0].revents & POLLIN;

       if (pollin_happened) {
	   printf("File descriptor %d is ready to read\n", pfds[0].fd);
       } else {
	   printf("Unexpected event occured: %d\n", pfds[0].revents);
       }
   }

   return 0;
}
