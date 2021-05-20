#include <string.h>
#include <stdio.h>
#include <bits/pthreadtypes.h>
#include <pthread.h>
#include <stdlib.h>
#include "commands.h"
#include "../responses.h"

uint32_t print_to_out(char *output, char *to_print) {
    if (output != NULL) {
        sprintf(output, "%s", to_print);
        uint32_t len = strlen(to_print);
        output[len] = '\0';
        return len + 1;
    } else {
        return -1;
    }

}

void lock(pthread_mutex_t *mutex) {
    if (mutex != NULL) {
        pthread_mutex_lock(mutex);
    }
}

void unlock(pthread_mutex_t *mutex) {
    if (mutex != NULL) {
        pthread_mutex_unlock(mutex);
    }
}

void notify_clients(uint32_t max_client, client_info *cl_info, char *to_send) {
    if (cl_info != NULL) {
        for (int i = 0; i < max_client; ++i) {
            if (!cl_info[i].t_is_free) {
                send(cl_info[i].notification_socket, to_send, strlen(to_send), MSG_NOSIGNAL);
            }
        }
    }
}

uint32_t handle_take(bookstore *store, pthread_mutex_t *mutex, client_info *cl_info, uint32_t max_client, char *to_notify,
                     char *output) {
    uint64_t book_num = strtol(strtok(NULL, "\0"), NULL, 10);
    book *found = &store->catalog[book_num];
    if (found != NULL) {
        lock(mutex);
        if (found->available > 0) {
            found->available--;
            notify_clients(max_client, cl_info, to_notify);
            unlock(mutex);
            return print_to_out(output, OK_RESP);
        } else {
            unlock(mutex);
            return print_to_out(output, NO_BOOKS_LEFT);
        }
    } else {
        return print_to_out(output, NOT_FOUND);
    }
}

uint32_t
handle_return(bookstore *store, pthread_mutex_t *mutex, client_info *cl_info, uint32_t max_client, char *to_notify,
              char *output) {
    uint64_t book_num = strtol(strtok(NULL, "\0"), NULL, 10);
    book *found = &store->catalog[book_num];
    if (found != NULL) {
        lock(mutex);
//        if (found->available < found->total) {
            if (found->available >= found->total) {
                found->total++;
            }
            found->available++;
            notify_clients(max_client, cl_info, to_notify);
            unlock(mutex);
            return print_to_out(output, OK_RESP);
//        } else {
//            unlock(mutex);
//            return print_to_out(output, STORAGE_IS_FULL);
//        }
    } else {
        return print_to_out(output, NOT_FOUND);
    }
}

uint32_t handle_add(bookstore *store, pthread_mutex_t *mutex, client_info *cl_info, uint32_t max_client, char *to_notify,
                    char *output) {
    lock(mutex);
    if (store->book_count < sizeof(store->catalog) / sizeof(store->catalog[0])) {
        char buf[50];
        sprintf(buf, "Untitled%u", store->book_count);
        strcpy(store->catalog[store->book_count++].title, buf);
        notify_clients(max_client, cl_info, to_notify);
        unlock(mutex);
        return print_to_out(output, OK_RESP);
    } else {
        unlock(mutex);
        return print_to_out(output, STORAGE_IS_FULL);
    }
}

uint32_t
handle_change(bookstore *store, pthread_mutex_t *mutex, client_info *cl_info, uint32_t max_client, char *to_notify,
              char *output) {
    char *num = strtok(NULL, " ");
    uint64_t book_num = strtol(num, NULL, 10);
    book *found = &store->catalog[book_num];
    if (found != NULL) {
        lock(mutex);
        char *to_change = strtok(NULL, " ");
        char *new_value = strtok(NULL, "\0");
        if (strcmp(to_change, "title") == 0) {
            strcpy(found->title, new_value);
            found->title[strlen(new_value)] = '\0';
        }
        if (strcmp(to_change, "author") == 0) {
            strcpy(found->author, new_value);
            found->author[strlen(new_value)] = '\0';
        }
        if (strcmp(to_change, "annotation") == 0) {
            strcpy(found->annotation, new_value);
            found->annotation[strlen(new_value)] = '\0';
        }
        notify_clients(max_client, cl_info, to_notify);
        unlock(mutex);
        return print_to_out(output, OK_RESP);
    } else {
        return print_to_out(output, NOT_FOUND);
    }
}

uint32_t handle_request(bookstore *ptr, pthread_mutex_t *mutex, client_info *cl_info, uint32_t max_client, char *input,
                        char *output) {
    char input_copy[1000] = {0};
    memset(input_copy, 0, 1000);
    strcpy(input_copy, input);
    char *command = strtok(input, " ");
    if (command != NULL) {
        if (strcmp(command, TEST) == 0) {
            return print_to_out(output, TEST);
        }
        if (strcmp(command, TAKE) == 0) {
            return handle_take(ptr, mutex, cl_info, max_client, input_copy, output);
        }
        if (strcmp(command, RETURN) == 0) {
            return handle_return(ptr, mutex, cl_info, max_client, input_copy, output);
        }
        if (strcmp(command, ADD) == 0) {
            return handle_add(ptr, mutex, cl_info, max_client, input_copy, output);
        }
        if (strcmp(command, CHANGE) == 0) {
            return handle_change(ptr, mutex, cl_info, max_client, input_copy, output);
        }
        if (strcmp(command, QUIT) == 0 || strcmp(command, EXIT) == 0 || strcmp(command, Q) == 0) {
            return print_to_out(output, OK_RESP);
        }
    }
    return print_to_out(output, WRONG_COMMAND);
}

//uint32_t handle_acceptable_command(bookstore *ptr, pthread_mutex_t *mutex, client_info *cl_info, uint32_t max_client, char *input,
//                                   char *output, char* input_copy, char* command){
//    lock(mutex);
//    uint32_t result;
//    if (strcmp(command, TAKE) == 0) {
//        result = handle_take(ptr, mutex, cl_info, max_client, input, output);
//    }
//    if (strcmp(command, RETURN) == 0) {
//        result =  handle_return(ptr, mutex, cl_info, max_client, input, output);
//    }
//    if (strcmp(command, ADD) == 0) {
//        result =  handle_add(ptr, mutex, cl_info, max_client, input, output);
//    }
//    if (strcmp(command, CHANGE) == 0) {
//        result =  handle_change(ptr, mutex, cl_info, max_client, input, output);
//    }
//    notify_clients(max_client, cl_info, input_copy);
//    unlock(mutex);
//    return result;
//} //fixme maybe use this