//
// Created by notnaturalselection on 05.05.2021.
//

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include "client.h"
#include "../responses.h"

int main(int argc, char **argv) {
    if (argc < 4) {
        puts("No required arguments: <hostname> <port> <login>");
        return -1;
    }
    pthread_t notification_t;

    int32_t server_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(strtol(argv[2], NULL, 10));
    server_address.sin_addr.s_addr = inet_addr(argv[1]);

    int connection = connect(server_fd, (const struct sockaddr *) &server_address, sizeof(server_address));
    if (connection == -1) {
        printf("There was an error making a connection to the remote socket \n");
        return -1;
    }

    char input[250];
    if (log_in(argv[3], server_fd) != 0) {
        puts("Client with this username is already logged in");
        return -1;
    }

    bookstore store;
    get_bookstore(server_fd, &store);
    client_arg arg;
    arg.store = &store;
    arg.server_fd = server_fd;
//    arg.log_stream = stdout;
    pthread_create(&notification_t, NULL, (void *(*)(void *)) manage_server_changes, &arg);

    while (strcmp(input, QUIT) != 0 && strcmp(input, EXIT) != 0 && strcmp(input, Q) != 0) {//TODO create UI and connect commands with UI actions
        fgets(input, sizeof(input), stdin);
        int len = strlen(input);
        input[len - 1] = '\0';
        send(server_fd, input, strlen(input), 0);
//        recv(server_fd, server_output, sizeof(server_output), 0);
//        puts(server_output);
    }

    close(server_fd);
}