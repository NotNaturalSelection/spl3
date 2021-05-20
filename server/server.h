#ifndef UNTITLED1_SERVER_H
#define UNTITLED1_SERVER_H

#include <netinet/in.h>
#include <stdbool.h>
#include <bits/types/FILE.h>
#include "../bookstore/bookstore.h"

typedef struct {
    int32_t client_socket;
    pthread_t t_id;
    int32_t notification_socket;
    bool t_is_free;
    char username[30];
} client_info;


typedef struct {
    uint16_t port;
    int32_t max_client_count;
    int32_t server_fd;
    pthread_t manager_t_id;
    client_info *clients_info;
    uint32_t curr_client_count;
    pthread_mutex_t client_count_mutex;
    FILE *log_stream;

    uint32_t
    (*req_handle_fun)(bookstore *ptr, pthread_mutex_t *mutex, client_info *, uint32_t max_client, char *, char *);

    bookstore *store;
} server_info;


server_info *
startup(uint16_t port, int32_t max_clients,
        uint32_t (*req_handler)(bookstore *ptr, pthread_mutex_t *mutex, client_info *cl_info, uint32_t max_client,
                                char *, char *), FILE *log_stream);

void close_server(server_info *info);

uint32_t handle_server_command(server_info *info, char *input, char *output);

_Noreturn void manage_connections(server_info *info);

#endif //UNTITLED1_SERVER_H
