//
// Created by notnaturalselection on 04.05.2021.
//
#ifndef UNTITLED1_COMMANDS_H
#define UNTITLED1_COMMANDS_H

#include <stdint.h>
#include "bookstore.h"
#include "../server/server.h"

uint32_t handle_request(bookstore *ptr, pthread_mutex_t *mutex, client_info * cl_info,  uint32_t max_client, char *input, char *output);

#endif //UNTITLED1_COMMANDS_H
