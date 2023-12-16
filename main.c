#include <stdio.h>

#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

#define port 9012
#define backlog 10 // backlog  argument  defines the maximum length to which the queue of pending connections for sockfd may grow.

void handle_client(int);

int main() {
    printf("Hello, this is me creating a http server only using the linux manual (for c code)!\n");

    // Create the socket
    
    /*
    * AF_INET      IPv4 Internet protocols
    * SOCK_STREAM     Provides  sequenced,  reliable,  two-way,  connection-based byte streams.  An out-of-band data transmission mechanism may be supported.
    * The protocol specifies a particular protocol to be used with the socket.  Normally only a single protocol  exists  to support a particular socket type 
    within a given protocol family, in which case protocol can be specified as 0.
    */ 
    int fd = socket(AF_INET, SOCK_STREAM, 0 );

    if (fd == -1) 
    {
        printf("Error creating socket.");
        return 1;
    }

   /*
    struct sockaddr_in {
        sa_family_t    sin_family; // address family: AF_INET
        in_port_t      sin_port;   // port in network byte order
        struct in_addr sin_addr;   // internet address
    };
   */
    struct sockaddr_in svr_addr;

    memset(&svr_addr, '0', sizeof(svr_addr));

    svr_addr.sin_family = AF_INET;
    svr_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    svr_addr.sin_port = htons(port);

    int binded = bind(fd, (struct sockaddr*) &svr_addr, sizeof(svr_addr));
    if (binded == -1)
    {
        printf("Error binding to socket.\n");
        return 1;
    }

    int listening = listen(fd, backlog);
    if (listening == -1)
    {
        printf("Error listening to socket.\n");
        return 1;
    }

    while (1)
    {
        struct sockaddr_in peer_addr;
        socklen_t peer_addr_size;

        int client_fd = accept(fd, (struct sockaddr*) &peer_addr, &peer_addr_size);
        if (client_fd == -1)
        {
            printf("Error handling client.\n");
        }
        else
        {
            handle_client(client_fd);
        }
    }

    return 0;
}

void handle_client(int client_fd) {
    printf("Handling client at file descriptor %d\n", client_fd);

    char buff[64];
    read(client_fd, buff, sizeof(buff));
    printf("%s\n", buff);

    bzero(buff, sizeof(buff));
}