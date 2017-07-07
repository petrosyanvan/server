#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFSIZE 1024

int main(int argc, char **argv) {
  int parentfd; /* parent socket */
  int childfd; /* child socket */
  int portno; /* port to listen on */
  socklen_t clientlen; /* byte size of client's address */
  struct sockaddr_in serveraddr; /* server's addr */
  struct sockaddr_in clientaddr; /* client addr */
  struct hostent *hostp; /* client host info */
  char buf[BUFSIZE]; /* message buffer */
  char *hostaddrp; /* dotted decimal host addr string */
  int optval; /* flag value for setsockopt */
  int n; /* message byte size */

  /*
   * check command line arguments
   */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  portno = atoi(argv[1]);

  /*
   * socket: create the parent socket
   */
  parentfd = socket(AF_INET, SOCK_STREAM, 0);
  if (parentfd < 0) {
    perror("ERROR opening socket");
    exit(EXIT_FAILURE);
  }

  /* setsockopt: Handy debugging trick that lets
   * us rerun the server immediately after we kill it;
   * otherwise we have to wait about 20 secs.
   * Eliminates "ERROR on binding: Address already in use" error.
   */
  optval = 1;
  setsockopt(parentfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));
  /*
   * build the server's Internet address
   */
  bzero((char *) &serveraddr, sizeof(serveraddr));
  /* this is an Internet address */
  serveraddr.sin_family = AF_INET;
  /* let the system figure out our IP address */
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
  /* this is the port we will listen on */
  serveraddr.sin_port = htons((unsigned short)portno);
  /*
   * bind: associate the parent socket with a port
   */
  if (bind(parentfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0){
    perror("ERROR on binding");
    exit(EXIT_FAILURE);
  }

  /*
   * listen: make this socket ready to accept connection requests
   */
  if (listen(parentfd, 5) < 0){ /* allow 5 requests to queue up */
    perror("ERROR on listen");
    exit(EXIT_FAILURE);
  }

  /*
   * main loop: wait for a connection request, echo input line,
   * then close connection.
   */
  clientlen = sizeof(clientaddr);
  while (1) {

    /*
     * accept: wait for a connection request
     */
    childfd = accept(parentfd, (struct sockaddr *) &clientaddr, &clientlen);
    if (childfd < 0){
      perror("ERROR on accept");
      exit(EXIT_FAILURE);
    }

    /*
     * gethostbyaddr: determine who sent the message
     */
    hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr,
			  sizeof(clientaddr.sin_addr.s_addr), AF_INET);
    if (hostp == NULL){
      perror("ERROR on gethostbyaddr");
      exit(EXIT_FAILURE);
    }
    hostaddrp = inet_ntoa(clientaddr.sin_addr);
    if (hostaddrp == NULL){
      perror("ERROR on inet_ntoa\n");
      exit(EXIT_FAILURE);
    }
    printf("server established connection with %s (%s)\n",
	   hostp->h_name, hostaddrp);

    /*
     * read: read input string from the client
     */
    bzero(buf, BUFSIZE);
    n = read(childfd, buf, BUFSIZE);
    if (n < 0){
      perror("ERROR reading from socket");
      exit(EXIT_FAILURE);
    }
    printf("server received %d bytes: %s", n, buf);

    /*
     * write: echo the input string back to the client
     */
    n = write(childfd, buf, strlen(buf));
    if (n < 0){
      perror("ERROR writing to socket");
      exit(EXIT_FAILURE);
    }

    close(childfd);
  }
}
