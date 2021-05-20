#ifndef UNTITLED1_UI_H
#define UNTITLED1_UI_H
#include <stdbool.h>
#include "ncurses.h"
typedef struct {
    char title[50];
    uint32_t title_len;
    char author[50];
    uint32_t author_len;
    char annotation[500];
    uint32_t annotation_len;
    char tags[50];
    uint32_t tags_len;
} main_win_fields;

typedef struct {
    WINDOW *list_win_border;
    WINDOW *header_win_border;
    WINDOW *main_win_border;
    WINDOW *footer_win_border;
    WINDOW *list_win;
    WINDOW *header_win;
    WINDOW *footer_win;
    WINDOW *main_win;
    uint32_t selected_book;
    uint32_t selected_book_attr;
    bool list_win_editing;
    bool edit_mode;
    char search_field[80];
    uint32_t search_field_len;
    book *buffer[100];
    uint32_t current_buffer_size;
    main_win_fields fields;
    bool (*search_fun)(book *, char*);
} UI;
#endif