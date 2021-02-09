/* server.c
 * Hensley, Robert
 * 
 * Description
 *     A TCP server that sends files in it's working directory when
 *     a client sends a request for it.
 *	
 *     usage: ./server [port number]
 * 
 * Specifications
 *     - all client transactions are output on the terminal
 *       and stored in a `log` text file
 *     - only has access to the files within the working directory
 *     - all dot files are illegal to access
 *     - sending the server executable is an illegal operation for the client
 * 	
 * Examples
 *     hostname -I 	    # record ip address for client to use
 *     ./server 4443 	# starts server on this port
 *     cat log			# view client / server transactions
 */

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
#include <dirent.h> 
#include <time.h>
#include <pthread.h>

#define BACKLOG 10 // how many pending connections queue will hold

#define USAGE_MESSAGE "Usage: server port # example port 4443\n"

void sigchld_handler(int s)
{
    (void)s; // quiet unused variable warning

    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while ( waitpid(-1, NULL, WNOHANG) > 0 );

    errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if ( sa->sa_family == AF_INET ) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void index_list(char * send_buf) {

    struct dirent *de;
    DIR * dr = opendir("."); 

    if (dr == NULL) {
        fprintf(stderr, "can't open server directory");
        exit(1);
    }

    while ((de = readdir(dr)) != NULL) {
        // remove '.' and '..' directory listings
        if(strcmp(".", de->d_name) && strcmp("..", de->d_name)) {
            strncat(send_buf, de->d_name, strlen(de->d_name));
            strncat(send_buf, "\n", 1);
        }
    } 

    closedir(dr);

}

void log_file(char * client, char * port, char * recv_buf, char * sb_bytes)
{

    char log_buf[BUFSIZ];
    FILE * fp;
    time_t t;
    struct tm tm;

    t = time(NULL);
    tm = *localtime(&t);

    sprintf(log_buf, "[%d-%02d-%02d %02d:%02d:%02d] %s:%s %s %s\n", 
        tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, 
        tm.tm_hour, tm.tm_min, tm.tm_sec,
        client, port, recv_buf, sb_bytes);

    // print log to server console
    // add this line for verbosity
    // printf(log_buf);

    // append log file with entry
    fp = fopen("log", "a");
    fputs(log_buf, fp);
    fclose(fp);

}

char * read_file(char * servername,
                 char * file_arg, 
                 char * ip,
                 char * port) 
{
    FILE * fp;
    char * file_contents;

    // dot file
    if (file_arg[0] == '.') {
        log_file(ip, port, file_arg, "BAD_FILENAME");
        exit(1);
    }

    // no filename given
    if (!strcmp(" ", file_arg)) {
        log_file(ip, port, "", "");
        exit(1);
    }

    // matches servername (illegal)
    // including this because argv[0] is './server'
    if (!strcmp("server", file_arg)) {
        log_file(ip, port, file_arg, "NOT_ALLOWED");
        exit(1);
    }

    // can't open file for reading
    if (!(fp = fopen(file_arg, "rb"))) {
        // do error logging here
        log_file(ip, port, file_arg, "NOT_FOUND");
        exit(1);
    }

    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    rewind(fp);

    file_contents = (char *) malloc(fsize + 1);
    fread(file_contents, 1, fsize, fp);
    fclose(fp);

    file_contents[fsize] = 0;

    return file_contents;
}

int main(int argc, char **argv)
{
    int sockfd, new_fd;  // listen on sockfd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    // check if port argument given (usually 4443)
    if ( argc != 2 ) {
        fprintf(stderr, USAGE_MESSAGE);
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ( (rv = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0 ) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ( (sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1 ) {
            perror("server: socket");
            continue;
        }

        if ( setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1 ) {
            perror("setsockopt");
            exit(1);
        }

        if ( bind(sockfd, p->ai_addr, p->ai_addrlen) == -1 ) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if ( p == NULL )  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    // listen allows queue of up to BACKLOG number
    if ( listen(sockfd, BACKLOG) == -1 ) {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if ( sigaction(SIGCHLD, &sa, NULL) == -1 ) {
        perror("sigaction");
        exit(1);
    }

    while ( 1 ) {  // main accept() loop
        sin_size = sizeof their_addr;

        // beginning of critical section (opening new_fd)
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if ( new_fd == -1 ) {
            perror("accept");
            continue;
        }

        // receive client IP address
        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);

        if ( !fork() ) {
            char recv_buf[1024] = { 0 };
            char * send_buf;
            char bytes[BUFSIZ];

            close(sockfd); // child doesn't need the listener

            recv(new_fd, recv_buf, sizeof(recv_buf)-1,0);

            if(!strcmp("index", recv_buf)) {
                send_buf = (char *) malloc(BUFSIZ);
                index_list(send_buf);
            } else {
                send_buf = read_file(argv[0], recv_buf, s, argv[1]);
            }

            // retrieve buffer bytes as string
            sprintf(bytes, "%zu", strlen(send_buf));
            // create log of it
            log_file(s, argv[1], recv_buf, bytes);

            if (send(new_fd, send_buf, strlen(send_buf), 0) == -1)
                perror("send");

            free(send_buf);
            close(new_fd);

            exit(0);
        }
        close(new_fd);  // parent doesn't need this

    }

    return 0;
}
