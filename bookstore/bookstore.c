//
// Created by notnaturalselection on 06.05.2021.
//

#include <string.h>
#include "bookstore.h"

bool by_title(book *ptr, char* to_find) {
    return strstr(ptr->title, to_find) != NULL;
}

bool by_annotation(book *ptr, char* to_find) {
    return strstr(ptr->annotation, to_find) != NULL;
}

bool by_author(book *ptr, char* to_find) {
    return strstr(ptr->author, to_find) != NULL;
}

bool by_tags(book*ptr, char* to_find) {
    for (int i = 0; i < ptr->tag_count; ++i) {
        if (strstr(ptr->tags[i], to_find) != NULL){
            return true;
        }
    }
    return false;
}

uint32_t search_book(bookstore *store, bool (*by)(book *, char*), char *to_find, book **buffer) {
    uint32_t count = 0;
    if (by != NULL) {
        for (int i = 0; i < store->book_count; ++i) {
            if (by(&store->catalog[i], to_find)) {
                buffer[count++] = &store->catalog[i];
            }
        }
    } else {
        for (int i = 0; i < store->book_count; ++i) {
//            if (by(&store->catalog[i], to_find)) {
                buffer[count++] = &store->catalog[i];
//            }
        }
    }
    return count;
}

uint16_t count_book_size(book *book1) {
    uint16_t size = sizeof(bookMap);
    size += strlen(book1->title);
    size += strlen(book1->annotation);
    size += strlen(book1->author);
    for (int i = 0; i < book1->tag_count; ++i) {
        size+= strlen(book1->tags[i]);
    }
    return size;
}

uint32_t count_bookstore_size(bookstore *store) {
    uint32_t size = 0;
    for (int i = 0; i < store->book_count; ++i) {
        size += count_book_size(&store->catalog[i]);
    }
    return size;
}

void unpack_bookstore(unsigned char *buf, uint32_t size, bookstore *store) {
    uint32_t estimated = 0;
    uint32_t book_count = 0;
    while (estimated < size) {
        bookMap book_map;
        memcpy(&book_map, buf + estimated, sizeof(bookMap));
        store->catalog[book_count].total = book_map.total;
        store->catalog[book_count].available = book_map.available;
        estimated+= sizeof(bookMap);
        memcpy(store->catalog[book_count].title, buf+estimated, book_map.title_size);
        estimated+=book_map.title_size;
        memcpy(store->catalog[book_count].author, buf+estimated, book_map.author_size);
        estimated+=book_map.author_size;
        memcpy(store->catalog[book_count].annotation, buf+estimated, book_map.annotation_size);
        estimated+=book_map.annotation_size;
        for (int i = 0; i < book_map.tag_count; ++i) {
            memcpy(store->catalog[book_count].tags[i], buf+estimated, book_map.tag_sizes[i]);
            estimated+=book_map.tag_sizes[i];
        }
        book_count++;
    }
    store->book_count = book_count;
}

void fill_book_map(book *book1, bookMap*book_map) {
    book_map->size = count_book_size(book1);
    book_map->available = book1->available;
    book_map->total = book1->total;
    book_map->tag_count = book1->tag_count;
    book_map->title_size = strlen(book1->title);
    book_map->author_size = strlen(book1->author);
    book_map->annotation_size = strlen(book1->annotation);
    for (int i = 0; i < book_map->tag_count; ++i) {
        book_map->tag_sizes[i] = strlen(book1->tags[i]);
    }
}

void pack_bookstore(unsigned char *buf, bookstore *store) { //buffer of exact needed size
    uint32_t rel_position = 0;
    for (int i = 0; i < store->book_count; ++i) {
//        unsigned char *byte_book_size = (unsigned char *) &book_size;
        bookMap book_map;
        fill_book_map(&store->catalog[i], &book_map);
        memcpy(buf + rel_position, &book_map, sizeof(bookMap));
        rel_position += sizeof(bookMap);
        memcpy(buf + rel_position, store->catalog[i].title, book_map.title_size);
        rel_position+= book_map.title_size;
        memcpy(buf + rel_position, store->catalog[i].author, book_map.author_size);
        rel_position+= book_map.author_size;
        memcpy(buf + rel_position, store->catalog[i].annotation, book_map.annotation_size);
        rel_position+= book_map.annotation_size;
        for (int j = 0; j <book_map.tag_count; ++j) {
            memcpy(buf + rel_position, store->catalog[i].tags[j], book_map.tag_sizes[j]);
            rel_position+= book_map.tag_sizes[j];
        }
    }
}

void init_store(bookstore *store) {
    store->book_count = 4;
    strcpy(store->catalog[0].title, "War and peace");
    strcpy(store->catalog[0].author, "Tolstoy L.N.");
    strcpy(store->catalog[0].annotation, "War and Peace, historical novel by Leo Tolstoy, originally published as Voyna i mir in 1865-69. This panoramic study of early 19th-century Russian society, noted for its mastery of realistic detail and variety of psychological analysis, is generally regarded as a masterwork of Russian literature and one of the world's greatest novels.");
    strcpy(store->catalog[0].tags[0], "classic");
    strcpy(store->catalog[0].tags[1], "novel");
    strcpy(store->catalog[0].tags[2], "historical");
    store->catalog[0].tag_count=3;
    store->catalog[0].available = 50;
    store->catalog[0].total = 100;

    strcpy(store->catalog[1].title, "Oblomov");
    strcpy(store->catalog[1].author, "Goncharov I.A.");
    strcpy(store->catalog[1].annotation,
           "This is a novel of will, or rather a novel studying the lack of will. Oblomov has a beautiful soul. He is capable of the most noble emotions. His intentions are always good. The storms aroused in his soul are genuine. They shake him deeply. He is honest, good-hearted, idealistically inclined. But, he is Oblomov. He is lazy. He is inertia incarnated. From the apparatus of his thoughts and emotions, there are no wires to the mechanism of action.");
    strcpy(store->catalog[1].tags[0], "classic");
    strcpy(store->catalog[1].tags[1], "novel");
    store->catalog[1].tag_count=2;
    store->catalog[1].available = 30;
    store->catalog[1].total = 67;

    strcpy(store->catalog[2].title, "Captain's daughter");
    strcpy(store->catalog[2].author, "Pushkin A.S.");
    strcpy(store->catalog[2].annotation,
           "The most famous Russian poet created not only excellent poems that have become classics, but also prose, no less vivid and interesting. The best works have long been a part of the school curriculum. The selected_book_attr \"The Captain's Daughter\" (Pushkin) also entered it. The description, plot, reviews and opinions of readers about this work - in our article.");
    strcpy(store->catalog[2].tags[0], "classic");
    strcpy(store->catalog[2].tags[1], "poems");
    strcpy(store->catalog[2].tags[2], "historical");
    store->catalog[2].tag_count=3;
    store->catalog[2].available = 90;
    store->catalog[2].total = 150;

    strcpy(store->catalog[3].title, "Fathers and sons");
    strcpy(store->catalog[3].author, "Turgenev I.S.");
    strcpy(store->catalog[3].annotation,
           "This work has been selected by scholars as being culturally important, and is part of the knowledge base of civilization as we know it. This work was reproduced from the original artifact, and remains as true to the original work as possible.");
    strcpy(store->catalog[3].tags[0], "classic");
    strcpy(store->catalog[3].tags[1], "novel");
    store->catalog[3].tag_count=2;
    store->catalog[3].available = 20;
    store->catalog[3].total = 45;
}