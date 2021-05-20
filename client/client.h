//
// Created by notnaturalselection on 05.05.2021.
//

#ifndef UNTITLED1_CLIENT_H
#define UNTITLED1_CLIENT_H

#include <stdint.h>
#include <bits/types/FILE.h>
#include "../bookstore/bookstore.h"
#include "ui/ui.h"

typedef struct {
    bookstore *store;
    int32_t server_fd;
//    FILE* log_stream;
    void(*draw_list)(UI *, bookstore*);
    UI *ui;
} client_arg;

int32_t log_in(char* login, int32_t server_fd);

void get_bookstore(int32_t server_fd, bookstore *store);

_Noreturn void manage_server_changes(client_arg*arg);

#endif //UNTITLED1_CLIENT_H
