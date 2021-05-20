#include <stdio.h>
#include <stdlib.h>
#include "server.h"
#include "../bookstore/commands.h"
#include "../responses.h"
#include <stdint.h>
#include <string.h>

#define BASE_10 10

int main(int argc, char **argv) {
    if (argc < 3) {
        puts("No required arguments provided: <server_port> <max_client_count>");
        return -1;
    }
    uint16_t port = strtoul(argv[1], NULL, BASE_10);
    int32_t listen_count = strtol(argv[2], NULL, BASE_10);

    server_info *info_ptr = startup(port, listen_count, handle_request, stdout);
    if (info_ptr!=NULL){
        init_store(info_ptr->store);
        char input[50];
        char output[1000];
        do {
            fgets(input, 50, stdin);
            input[strlen(input)-1] = '\0';
            handle_server_command(info_ptr, input, output);
            puts(output);
        } while (strcmp(input, QUIT) != 0);
        close_server(info_ptr);
        free(info_ptr);
    } else {
        puts("Something went wrong");
    }
}

//TODO
//1 создавать книги с именем untitledN, где N - номер последовательности
//2 передавать из recv клиента сигналы на ui для переотрисовки
//3 сделать поля изменяемыми in-place (добавить по буферу для каждого и отрисовывать как в search)
