//
// Created by notnaturalselection on 05.05.2021.
//

#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include "client.h"
#include "../responses.h"
#include "../bookstore/commands.h"


int32_t log_in(char* login, int32_t server_fd) {
    char server_out[50];
    send(server_fd, login, strlen(login), 0);
    recv(server_fd, server_out, sizeof(server_out), 0);
    return strcmp(server_out, OK_RESP);
}

void get_bookstore(int32_t server_fd, bookstore *store){
    uint32_t size;
    recv(server_fd, &size, sizeof(uint32_t), 0);
    unsigned char server_out[size];
    recv(server_fd, server_out, size, 0);
    unpack_bookstore(server_out, size, store);
}

_Noreturn void manage_server_changes(client_arg*arg) {
    char input[500];
    while (1) {
        memset(input, 0, 500);
        recv(arg->server_fd, input, 500, MSG_NOSIGNAL);
//        fprintf(arg->log_stream, "%s\n", input);
        handle_request(arg->store, NULL, NULL, 0, input, NULL);
        arg->draw_list(arg->ui, arg->store);//fixme when updates
        wrefresh(arg->ui->list_win);
        wrefresh(arg->ui->main_win);
    }
}
