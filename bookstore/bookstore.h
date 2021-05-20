#ifndef UNTITLED1_BOOKSTORE_H
#define UNTITLED1_BOOKSTORE_H

#include <bits/types/time_t.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    char title[50];
    char annotation[500];
    char author[50];
    uint32_t total;
    uint32_t available;
    uint16_t tag_count;
    char tags[10][20];
} book;

typedef struct {
    uint16_t size;
    uint32_t available;
    uint32_t total;
    uint8_t title_size;
    uint8_t author_size;
    uint16_t annotation_size;
    uint16_t tag_count;
    uint8_t tag_sizes[10];
} bookMap;

union book_data {
    char ch[26];
    bookMap book_map;
};

typedef struct {
    book catalog[100];
    uint32_t book_count;
} bookstore;

bookstore *load_from_file(char *filename);

uint32_t save_to_file(char *filename, bookstore *ptr);

bool by_title(book *ptr, char* to_find);
bool by_annotation(book *ptr, char* to_find);
bool by_author(book *ptr, char* to_find);
bool by_tags(book *ptr, char* to_find);

//book *find_book(bookstore *store, bool (*by)(book *, char*), char *to_find);

uint32_t search_book(bookstore *store, bool (*by)(book *, char*), char *to_find, book **buffer);

void pack_bookstore(unsigned char *buf, bookstore *store);

void unpack_bookstore(unsigned char *buf, uint32_t size, bookstore *store);

uint32_t count_bookstore_size(bookstore *store);

void init_store(bookstore *store);

#endif //UNTITLED1_BOOKSTORE_H
