/*
 ============================================================================
 Name        : server.c
 Author      : Van Petrosyan
 Version     :
 Copyright   : Your copyright notice
 Description : server in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <termios.h>
#include <pty.h>
#include <errno.h>
#include <signal.h>
#include <sys/poll.h>

#define BUFSIZE 		1024

int main(int argc, char **argv) {
  int parentfd; /* parent socket */
  int childfd; /* child socket */
  int portno; /* port to listen on */
  socklen_t clientlen; /* byte size of client's address */
  struct sockaddr_in serveraddr; /* server's addr */
  struct sockaddr_in clientaddr; /* client addr */
  struct hostent *hostp; /* client host info */
  char *hostaddrp; /* dotted decimal host addr string */
  int optval; /* flag value for setsockopt */
  int pty;

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
  memset((char *) &serveraddr, 0, sizeof(serveraddr));
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

  clientlen = sizeof(clientaddr);
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
  hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr, sizeof(clientaddr.sin_addr.s_addr), AF_INET);
  if (hostp == NULL){
    perror("ERROR on gethostbyaddr");
    exit(EXIT_FAILURE);
  }
  hostaddrp = inet_ntoa(clientaddr.sin_addr);
  if (hostaddrp == NULL){
    perror("ERROR on inet_ntoa\n");
    exit(EXIT_FAILURE);
  }
  printf("server established connection with %s (%s)\n", hostp->h_name, hostaddrp);
  pid_t pid = forkpty(&pty, NULL, NULL, NULL);
  if(pid == -1){
    perror("Couldn't fork\n");
    exit(EXIT_FAILURE);
  }
  else if (pid == 0){ // if child, exec process
  	close(childfd);
  	exit(execlp("/bin/bash", "/bin/bash", NULL));
  }
  //pty_scriptfoo(pty);
  char c;
  fd_set descriptor_set;

  while(1){
	  FD_ZERO (&descriptor_set);
	  FD_SET (childfd, &descriptor_set);
	  FD_SET (pty, &descriptor_set);

	  if (select (FD_SETSIZE, &descriptor_set, NULL, NULL, NULL) < 0) {
		  perror ("select()");
		  exit (EXIT_FAILURE);
	  }

	  if (FD_ISSET (childfd, &descriptor_set)) {
		  if ( (read (childfd, &c, 1) != 1) || (write (pty, &c, 1) != 1) ) {
			  fprintf (stderr, "Disconnected\n");
			  exit (EXIT_FAILURE);
		  }
	  }

	  if (FD_ISSET (pty, &descriptor_set)) {
		  if ((read (pty, &c, 1) != 1) || (write (childfd, &c, 1) != 1)) {
			  fprintf (stderr, "Disconnected\n");
			  exit (EXIT_FAILURE);
		  }
	  }
  }
  return EXIT_SUCCESS;
}