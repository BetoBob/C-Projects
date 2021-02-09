/* client.c
 * Hensley, Robert
 * 
 * Description
 *     A TCP client that requests files from a server and 
 *     outputs the files contents if successfully received.
 * 
 *     usage: ./client [ip]:[port] [optional: filename / command]
 * 
 * Specifications
 *     - use the command `index` to see the working directory
 *       of the server
 *     - only outputs files that exist on the server and that
 *       can be successfully opened
 * 
 * Examples
 *     $ ./client 129.65.128.82:4443 testfile
 *     This is a testfile.
 *     $ ./client 129.65.128.82:4443 log
 *     ...entire contents of log file should appear here...
 *     $ ./client 129.65.128.82:4443 doesnotexist
 *     $ ./client 129.65.128.82:4443 index
 *     server
 *     .dotfile
 *     emptyfile
 *     testfile
 *     bigfile
 *     log
 *     bad?filename
 *     notreadablefile
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#define USAGE_MESSAGE "Usage: client ipaddr:port # example 129.65.128.82:4443 [filename]\n"

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if ( sa->sa_family == AF_INET ) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void split_addr(char * arg, char * ipaddr, char * port) 
{
    // copy ippaddr
    strcpy(ipaddr, strsep(&arg, ":"));

    // check if colon exists
    if (arg == NULL) {
        fprintf(stderr, USAGE_MESSAGE);
        exit(1);
    }

    // copy port
    strcpy(port, arg);
}

int main(int argc, char **argv)
{
    int sockfd, numbytes;  
    char buf[BUFSIZ];
    struct addrinfo hints, *servinfo, *p;
    int rv;

    char ipaddr[50];
    char port[50];

    // check arguments
    if ( argc < 2 || argc > 3 ) {
        fprintf(stderr, USAGE_MESSAGE);
        exit(1);
    }

    // splits the ipaddr and port
    split_addr(argv[1], ipaddr, port);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ( (rv = getaddrinfo(ipaddr, port, &hints, &servinfo)) != 0 ) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ( (sockfd = socket(p->ai_family,
                          p->ai_socktype,
                          p->ai_protocol)) == -1 ) {
            perror("client: socket");
            continue;
        }

        if ( connect(sockfd, p->ai_addr, p->ai_addrlen) == -1 ) {
            perror("client: connect");
            close(sockfd);
            continue;
        }

        break;
    }

    if ( p == NULL ) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    freeaddrinfo(servinfo); // all done with this structure
  
    if(argc == 3)
        strcpy(buf, argv[2]);
    else 
        strcpy(buf, " "); // fix this

    // send argv[2] to the server
    send(sockfd, buf, strlen(buf), 0);

    if ( (numbytes = recv(sockfd, buf, sizeof(buf)-1, 0)) == -1 ) {
        perror("recv");
        exit(1);
    }

    buf[numbytes] = '\0';

    fputs(buf, stdout);

    close(sockfd);

    return 0;
}
