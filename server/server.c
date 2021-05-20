#include "server.h"
#include "../responses.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <pthread.h>

#define REQUEST_BUF_SIZE 100
#define RESPONSE_BUF_SIZE 1000

server_info *
create_server_info(uint16_t port, int32_t listen_count,
                   uint32_t (*req_handle_fun)(bookstore *, pthread_mutex_t *, client_info *, uint32_t ,
                                              char *, char *)) {
    server_info *server_info_ptr = malloc(sizeof(server_info));
    server_info_ptr->max_client_count = listen_count;
    server_info_ptr->port = port;
    server_info_ptr->req_handle_fun = req_handle_fun;
    server_info_ptr->clients_info = malloc(sizeof(client_info) * listen_count);
    for (int i = 0; i < listen_count; ++i) {
        server_info_ptr->clients_info[i].t_is_free = true;
    }
    pthread_mutex_init(&server_info_ptr->client_count_mutex, NULL);
    return server_info_ptr;
}

struct accept_client_arg {
    int32_t client_num;
    server_info *srv_info;
};

void
work_with_client(int32_t client_socket, server_info *ptr,
                 uint32_t (*req_handle_fun)(bookstore *, pthread_mutex_t *, client_info *, uint32_t , char *,
                                            char *)) {
    char buffer[REQUEST_BUF_SIZE];
    char response_buf[RESPONSE_BUF_SIZE];
    do {
        memset(buffer, 0, REQUEST_BUF_SIZE);
        memset(response_buf, 0, RESPONSE_BUF_SIZE);
        recv(client_socket, buffer, REQUEST_BUF_SIZE, MSG_NOSIGNAL);
        /*uint32_t response_size = */req_handle_fun(ptr->store, &ptr->client_count_mutex, ptr->clients_info,
                                                ptr->max_client_count, buffer,
                                                response_buf);
//        send(client_socket, response_buf, response_size, MSG_NOSIGNAL); //maybe fixme
    } while (strcmp(buffer, QUIT) != 0 && strcmp(buffer, EXIT) != 0 && strcmp(buffer, Q) != 0);
}

void send_packed_bookstore(struct accept_client_arg *arg) {
    uint32_t pack_size = count_bookstore_size(arg->srv_info->store);
    send(arg->srv_info->clients_info[arg->client_num].client_socket, &pack_size, sizeof(uint32_t), MSG_NOSIGNAL);
    unsigned char buf[pack_size];
    pack_bookstore(buf, arg->srv_info->store);
    send(arg->srv_info->clients_info[arg->client_num].client_socket, buf, pack_size, MSG_NOSIGNAL);
}

void accept_client(struct accept_client_arg *arg) {
    send_packed_bookstore(arg);

    work_with_client(arg->srv_info->clients_info[arg->client_num].client_socket, arg->srv_info,
                     arg->srv_info->req_handle_fun);

    pthread_mutex_lock(&arg->srv_info->client_count_mutex);
    arg->srv_info->clients_info[arg->client_num].t_is_free = true;
    arg->srv_info->curr_client_count--;
    fprintf(arg->srv_info->log_stream, "%s has logged out\n", arg->srv_info->clients_info[arg->client_num].username);
    pthread_mutex_unlock(&arg->srv_info->client_count_mutex);
    close(arg->srv_info->clients_info[arg->client_num].client_socket);
    free(arg);
}

int32_t find_free_pos(server_info *info_ptr) {
    pthread_mutex_lock(&info_ptr->client_count_mutex);
    for (int32_t i = 0; i < info_ptr->max_client_count; ++i) {
        if (info_ptr->clients_info[i].t_is_free) {
            info_ptr->clients_info[i].t_is_free = false;
            info_ptr->curr_client_count++;
            pthread_mutex_unlock(&info_ptr->client_count_mutex);
            return i;
        }
    }
    pthread_mutex_unlock(&info_ptr->client_count_mutex);
    return -1;
}

int32_t expand_clients(server_info*info) {
    client_info *current = info->clients_info;
    int32_t current_size = info->max_client_count;
    client_info *new = malloc(sizeof(client_info)*current_size*2);
    memcpy(new, current, sizeof(client_info)*current_size);
    free(current);
    info->clients_info = new;
    for (int i = current_size; i < current_size*2; ++i) {
        info->clients_info[i].t_is_free = true;
    }
    info->max_client_count = current_size*2;
    return current_size*2;
}

bool is_logged(server_info *info, char *username) {
    for (int i = 0; i < info->max_client_count; ++i) {
        if (!info->clients_info[i].t_is_free && strcmp(info->clients_info[i].username, username) == 0) {
            return true;
        }
    }
    return false;
}

int32_t handle_login(server_info *info, int32_t socket, char *username) {
    char buf[50];
    recv(socket, buf, 50, MSG_NOSIGNAL);
    if (is_logged(info, buf)) {
        send(socket, ALREADY_LOGGED, sizeof(ALREADY_LOGGED), 0);
        return -1;
    } else {
        send(socket, OK_RESP, sizeof(OK_RESP), 0);
        sprintf(username, "%s", buf);
        return 0;
    }
}


_Noreturn void manage_connections(server_info *info) {
    while (1) {
        if (info->curr_client_count < info->max_client_count) {
            struct sockaddr_in client_addr;
            socklen_t addr_len = sizeof(client_addr);
            int32_t accepted_socket = accept(info->server_fd, (struct sockaddr *) &client_addr, &addr_len);
            char username[50];
            if (handle_login(info, accepted_socket, username) == -1) {
                close(accepted_socket);
                continue;
            }
            struct accept_client_arg *arg = malloc(sizeof(struct accept_client_arg));
            arg->client_num = find_free_pos(info);
            arg->srv_info = info;
            arg->srv_info->clients_info[arg->client_num].client_socket = accepted_socket;
            arg->srv_info->clients_info[arg->client_num].notification_socket = accepted_socket;

            strcpy(arg->srv_info->clients_info[arg->client_num].username, username);
            fprintf(info->log_stream, "Client %d logged in as %s\n", arg->client_num, username);
//            accepted_socket = accept(info->server_fd, (struct sockaddr *) &client_addr, &addr_len);
//            arg->srv_info->clients_info[arg->client_num].notification_socket = accepted_socket; //maybe fixme
            pthread_create(&info->clients_info[arg->client_num].t_id, NULL, (void *(*)(void *)) accept_client, arg);
        } else {
            expand_clients(info);
//            sleep(1);
        }
    }
}


server_info *
startup(uint16_t port, int32_t max_clients,
        uint32_t (*req_handler)(bookstore *, pthread_mutex_t *, client_info *, uint32_t , char *, char *),
        FILE *log_stream

) {
    server_info *server_info_ptr = create_server_info(port, max_clients, req_handler);
    int created_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_addr.
            s_addr = htonl(INADDR_ANY);
    addr.
            sin_port = htons(server_info_ptr->port);
    addr.
            sin_family = AF_INET;
    server_info_ptr->
            server_fd = created_socket;
    server_info_ptr->
            log_stream = log_stream;
    int bind_result = bind(created_socket, (const struct sockaddr *) &addr, sizeof(addr));
    if (bind_result == -1) {
        return NULL;
    }
    server_info_ptr->
            store = malloc(sizeof(bookstore));
    listen(created_socket,
           1);
    pthread_create(&server_info_ptr->manager_t_id, NULL, (void *(*)(void *)) manage_connections, server_info_ptr);
    fprintf(log_stream,
            "Server successfully started on %d\n",
            ntohs(addr
                          .sin_port));
    return
            server_info_ptr;
}

uint32_t handle_server_command(server_info *info, char *input, char *output) {
    uint32_t written = 0;
    if (strcmp(input, "list") == 0) {
        for (int i = 0; i < info->curr_client_count; ++i) {
            char buf[50];
            written += sprintf(buf, "%2d %s\n", i, info->clients_info[i].username);
            strcat(output, buf);
        }
        return written;
    }
    if (strcmp(input, QUIT) == 0) {
        memset(output, 0, 1);
        return 4;
    }
    sprintf(output, WRONG_COMMAND);
    return strlen(WRONG_COMMAND);
}

void close_server(server_info *info) {
    close(info->server_fd);
}