#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

// For debugging
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

#define PORT 9042
#define BACKLOG 10 // BACKLOG  argument  defines the maximum length to which the queue of pending connections for sockfd may grow.

#define MAX_PACKET_SIZE 8192

void handle_client(int);

int get_request_line_count(char* buff);
char* parse_method(char *buff);
char* parse_endpoint(char* buff);

int main() {
    printf("Hello, this is me creating a http server only using the linux manual (for c code)!\n");
    printf("Host: 127.0.0.1:%d\n", PORT);

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
        in_port_t      sin_port;   // PORT in network byte order
        struct in_addr sin_addr;   // internet address
    };
   */
    struct sockaddr_in svr_addr;

    memset(&svr_addr, '0', sizeof(svr_addr));

    svr_addr.sin_family = AF_INET;
    svr_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    svr_addr.sin_port = htons(PORT);

    int binded = bind(fd, (struct sockaddr*) &svr_addr, sizeof(svr_addr));
    if (binded == -1)
    {
        printf("Error binding to socket.\n");
        return 1;
    }

    int listening = listen(fd, BACKLOG);
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
            printf("Error starting client handler.\n");
        }
        else
        {
            handle_client(client_fd);
        }
    }

    return 0;
}

/*
Request syntax:

L1: <METHOD> <URI> <PROTOCOL>
L2-(n-1): hkey: hv
l(n): nl
*/

int get_request_line_count(char *buff) {
    int linecount = 0;
    size_t char_count = strlen(buff);
    for (size_t i = 0; i < char_count; i++) {
        {
            if (buff[i] == '\n')
                linecount++;
        }
    }
    return linecount;
}

char* parse_method(char *buff) {
    // Method is the first thing sent, it will simply terminate at the first space.
    // To define the char array we first need to determine the size of the method name.
    const int max_method_size = 32;

    char *method = malloc(max_method_size);

    for (int i = 0; i < max_method_size; i++) {
        if (buff[i] == ' ')
            break;
        else
            method[i] = buff[i];
    }

    return method;
}

char* parse_endpoint(char *buff) {
    // endpoint is the second thing sent, first we need to find the first space as a starting index.
    int start_idx = 1;
    size_t char_count = strlen(buff);

    for (size_t i = 0; i < char_count; i++) {
        if (buff[i] == ' ')
            break;
        start_idx++;
    }
    // Next we need to find the end of the endpoint.
    int end_idx = start_idx;
    for (size_t i = start_idx; i < char_count; i++) {
        if (buff[i] == ' ')
            break;
        end_idx++;
    }

    // Now we know the range the endpoint is in is start_idx:end_idx.
    char *endpoint = malloc(end_idx-start_idx);
    
    for (int i = start_idx; i < end_idx; i++)
    {
        endpoint[i-start_idx] = buff[i];
    }

    return endpoint;
}

void parse_headers(char *buff, char* header_results[MAX_PACKET_SIZE][2]) {
    printf("Parsing headers.\n");

    char *lines = NULL;

    lines = strtok(buff, "\r\n");

    int line = 0;
    while (1)
    {
        lines = strtok(NULL, "\r\n");
        if (lines == NULL)
            break;
        
        int line_len = strlen(lines);
        
        char *key = malloc(line_len);
        char *value = malloc(line_len);

        int seperator_idx = -1;
        for(int i = 0; i < line_len; i++)
        {
            if (seperator_idx == -1 && lines[i] == ':')
                seperator_idx = i;

            if (seperator_idx == -1)
                key[i] = lines[i];
            else
                value[i-seperator_idx] = lines[i];
        }
        value += 2; // Remove the `: `

        header_results[line][0] = key;
        header_results[line][1] = value;
        line++;
    }
}

void handle_client(int client_fd) {
    printf("Handling client at file descriptor %d\n", client_fd);

    char buff[MAX_PACKET_SIZE];
    read(client_fd, buff, sizeof(buff));

    printf("REQUEST:\n---\n%s---\n", buff);

    int lc = get_request_line_count(buff);
    printf("Line count: %d\n", lc);

    char* method = parse_method(buff);
    printf("Request method: %s\n", method);

    char* endpoint = parse_endpoint(buff);
    printf("Request endpoint: %s\n", endpoint);

    char* headers[MAX_PACKET_SIZE][2];
    parse_headers(buff, headers);

    printf("Request headers:\n");
    for (size_t i = 0; i < sizeof(headers); i++)
    {
        if (headers[i][0])
            printf("%s: %s\n", headers[i][0], headers[i][1]);
        else
            break;
    }
}
