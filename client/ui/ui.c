//
// Created by notnaturalselection on 16.05.2021.
//
#include <ncurses.h>
#include <panel.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include "../../bookstore/bookstore.h"
#include "../client.h"
#include "ui.h"
#include "pthread.h"
#include "../../responses.h"

#define SEARCH_PREFIX "Search:%s"
#define TITLE_STR 0
#define AUTHOR_STR 1
#define TAG_STR 2
#define ANNOTATION_STR 3


//void init_store(bookstore *store) {
//    store->book_count = 4;
//    strcpy(store->catalog[0].title, "War and peace");
//    strcpy(store->catalog[0].author, "Tolstoy L.N.");
//    strcpy(store->catalog[0].annotation, "\n"
//                                         "War and Peace, historical novel by Leo Tolstoy, originally published as Voyna i mir in 1865-69. This panoramic study of early 19th-century Russian society, noted for its mastery of realistic detail and variety of psychological analysis, is generally regarded as a masterwork of Russian literature and one of the world's greatest novels.");
//    store->catalog[0].available = 50;
//    store->catalog[0].total = 100;
//
//    strcpy(store->catalog[1].title, "Oblomov");
//    strcpy(store->catalog[1].author, "Goncharov");
//    strcpy(store->catalog[1].annotation,
//           "This is a novel of will, or rather a novel studying the lack of will. Oblomov has a beautiful soul. He is capable of the most noble emotions. His intentions are always good. The storms aroused in his soul are genuine. They shake him deeply. He is honest, good-hearted, idealistically inclined. But, he is Oblomov. He is lazy. He is inertia incarnated. From the apparatus of his thoughts and emotions, there are no wires to the mechanism of action.");
//    store->catalog[1].available = 30;
//    store->catalog[1].total = 67;
//
//    strcpy(store->catalog[2].title, "Captain's daughter");
//    strcpy(store->catalog[2].author, "Pushkin A.S.");
//    strcpy(store->catalog[2].annotation,
//           "The most famous Russian poet created not only excellent poems that have become classics, but also prose, no less vivid and interesting. The best works have long been a part of the school curriculum. The selected_book_attr \"The Captain's Daughter\" (Pushkin) also entered it. The description, plot, reviews and opinions of readers about this work - in our article.");
//    store->catalog[2].available = 90;
//    store->catalog[2].total = 150;
//
//    strcpy(store->catalog[3].title, "Fathers and sons");
//    strcpy(store->catalog[3].author, "Turgenev I.S.");
//    strcpy(store->catalog[3].annotation,
//           "This work has been selected by scholars as being culturally important, and is part of the knowledge base of civilization as we know it. This work was reproduced from the original artifact, and remains as true to the original work as possible.");
//    store->catalog[3].available = 20;
//    store->catalog[3].total = 45;
//}


uint32_t copy_buffer_from_store(bookstore *store, book **buffer) {
    uint32_t count = store->book_count;
    for (int i = 0; i < count; ++i) {
        buffer[i] = &store->catalog[i];
    }
    return count;
}

WINDOW *init_win(int width, int height, int woffset, int hoffset, bool is_border) {
    WINDOW *win = newwin(width, height, woffset, hoffset);
    wattron(win, A_NORMAL);
    wattrset(win, COLOR_PAIR(1));
    if (is_border) {
        box(win, 0, 0);
    }
    return win;
}

void print_colored(WINDOW *win, int32_t x, int32_t y, char *pattern, char *arg, bool colored) {
    if (colored) {
        wcolor_set(win, 2, NULL);
    }
    mvwprintw(win, x, y, pattern, arg);
    wcolor_set(win, 1, NULL);
}

void init_wins(UI *ui) {
    int cols03 = COLS * 0.3;
    ui->list_win_border = init_win(LINES - 7, cols03, 3, 0, true);
    ui->list_win = init_win(LINES - 9, cols03 - 2, 4, 1, false);

    ui->header_win_border = init_win(3, COLS, 0, 0, true);
    ui->header_win = init_win(1, COLS - 2, 1, 1, false);
    mvwprintw(ui->header_win, 0, 0, SEARCH_PREFIX, " ");
    ui->main_win_border = init_win(LINES - 7, COLS - cols03, 3, cols03, true);
    ui->main_win = init_win(LINES - 9, COLS - cols03 - 2, 4, cols03 + 1, false);

    ui->footer_win_border = init_win(4, COLS, LINES - 4, 0, true);
    ui->footer_win = init_win(2, COLS - 2, LINES - 3, 1, false);
    mvwprintw(ui->footer_win, 0, 0,
              "Take[F1] Return[F2] Add[F3] Edit[F8] Quit[F12]"); //Search by: Title[F4] Author[F5] Annotation[F6] Tags[F7]
    mvwprintw(ui->footer_win, 1, 0, "Search by: Title[F4] Author[F5] Annotation[F6] Tags[F7]");
}

void refresh_all(UI *ui, bool borders, bool main) {
    if (borders) {
        wrefresh(ui->footer_win_border);
        wrefresh(ui->list_win_border);
        wrefresh(ui->header_win_border);
        wrefresh(ui->main_win_border);
    }
    if (main) {
        wrefresh(ui->list_win);
        wrefresh(ui->header_win);
        wrefresh(ui->main_win);
        wrefresh(ui->footer_win);
    }
}

void redraw(WINDOW *win) {
    wclear(win);
    wrefresh(win);
}

void construct_tag_str(book*book1, char* tagbuf){
    sprintf(tagbuf, "%s", book1->tags[0]);
    if (book1->tag_count > 1) {
        for (int i = 1; i < book1->tag_count; ++i) {
            strcat(tagbuf, ", ");
            strcat(tagbuf, book1->tags[i]);
        }
    }
}

void draw_book_info(book *book1, UI *ui, bool init) {
        wclear(ui->main_win);
    if (init) {
        sprintf(ui->fields.title, "%s", book1->title);
        ui->fields.title_len = strlen(book1->title);
        sprintf(ui->fields.author, "%s", book1->author);
        ui->fields.author_len = strlen(book1->author);
        char tagbuf[50] = {0};
        if (book1->tag_count > 0) {
            construct_tag_str(book1, tagbuf);
        }
        sprintf(ui->fields.tags, "%s", tagbuf);
        ui->fields.tags_len = strlen(tagbuf);

        sprintf(ui->fields.annotation, "%s", book1->annotation);
        ui->fields.annotation_len = strlen(book1->annotation);
    }
    if (!ui->list_win_editing) {
        print_colored(ui->main_win, TITLE_STR, 0, "Title: %s", ui->fields.title, TITLE_STR == ui->selected_book_attr);
        print_colored(ui->main_win, AUTHOR_STR, 0, "Author: %s", ui->fields.author, AUTHOR_STR == ui->selected_book_attr);
        print_colored(ui->main_win, TAG_STR, 0, "Tags: %s", ui->fields.tags,TAG_STR == ui->selected_book_attr);
        print_colored(ui->main_win, ANNOTATION_STR, 0, "Annotation: %s", ui->fields.annotation,
                      ANNOTATION_STR == ui->selected_book_attr);
        mvwprintw(ui->main_win, getmaxy(ui->main_win) - 1, 0, "Available: %u/%u", book1->available, book1->total);
    } else {
        mvwprintw(ui->main_win, TITLE_STR, 0, "Title: %s", ui->fields.title);
        mvwprintw(ui->main_win, AUTHOR_STR, 0, "Author: %s", ui->fields.author);
        mvwprintw(ui->main_win, TAG_STR, 0, "Tags: %s", ui->fields.tags);
        mvwprintw(ui->main_win, ANNOTATION_STR, 0, "Annotation: %s", ui->fields.annotation);
        mvwprintw(ui->main_win, getmaxy(ui->main_win) - 1, 0, "Available: %u/%u", book1->available, book1->total);
    }

}

void draw_search_field(UI *ui) {
    redraw(ui->header_win);
    mvwprintw(ui->header_win, 0, 0, SEARCH_PREFIX, ui->search_field);
}

void draw_list(UI *ui, bookstore *store) {
    redraw(ui->list_win);
    ui->current_buffer_size = search_book(store, ui->search_fun, ui->search_field, ui->buffer);
    for (int i = 0; i < ui->current_buffer_size; ++i) {
        if (ui->selected_book == i) {
            wcolor_set(ui->list_win, 2, NULL);
        }
        mvwprintw(ui->list_win, i, 0, "%s", ui->buffer[i]->title);
        if (ui->selected_book == i) {
            wcolor_set(ui->list_win, 1, NULL);
            redraw(ui->main_win);
            draw_book_info(ui->buffer[i], ui, true);
        }
    }
}

void init_ui(bookstore*store, UI*ui){
    init_store(store);//fixme test data
    initscr();
    noecho();
    curs_set(0);
    keypad(stdscr, true);
    start_color();
    refresh();

    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_BLACK, COLOR_WHITE);

    init_wins(ui);
    ui->selected_book = 0;
    ui->selected_book_attr = 0;
    ui->search_field_len = 0;
    ui->list_win_editing = true;
    ui->edit_mode = false;
    for (int i = 0; i < sizeof(ui->search_field) / sizeof(ui->search_field[0]); ++i) {
        ui->search_field[i] = '\0';
    }
    ui->current_buffer_size = copy_buffer_from_store(store, ui->buffer);
    draw_list(ui, store);
    refresh_all(ui, true, true);
}

char* get_attr_by_num(uint32_t num){
    switch (num) {
        case TITLE_STR:
            return "title";
        case AUTHOR_STR:
            return "author";
        case TAG_STR:
            return "tags";
        case ANNOTATION_STR:
            return "annotation";
    }
}

char* get_attr_value_by_num(uint32_t num, main_win_fields*fields){
    switch (num) {
        case TITLE_STR:
            return fields->title;
        case AUTHOR_STR:
            return fields->author;
        case TAG_STR:
            return fields->tags;
        case ANNOTATION_STR:
            return fields->annotation;
    }
}

int main(int argc, char **argv) {
    if (argc < 4) {
        puts("No required arguments: <hostname> <port> <login>");
        return -1;
    }
    pthread_t notification_t;

    int32_t server_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(strtol(argv[2], NULL, 10));
    server_address.sin_addr.s_addr = inet_addr(argv[1]);

    int connection = connect(server_fd, (const struct sockaddr *) &server_address, sizeof(server_address));
    if (connection == -1) {
        printf("There was an error making a connection to the remote socket \n");
        return -1;
    }

    char input[250];
    if (log_in(argv[3], server_fd) != 0) {
        puts("Client with this username is already logged in");
        return -1;
    }


    bookstore store;
    get_bookstore(server_fd, &store);
    UI ui;
    init_ui(&store, &ui);
    client_arg arg;
    arg.store = &store;
    arg.server_fd = server_fd;
    arg.ui = &ui;
    arg.draw_list = draw_list;
    pthread_create(&notification_t, NULL, (void *(*)(void *)) manage_server_changes, &arg);

    int cur = getch();
    while (cur != KEY_F(12) /*'q'*/) {
        char command[500] = {0};
        memset(command, 0, 500);
        switch (cur) {
            case KEY_DOWN:
                if (ui.list_win_editing) {
                    if (ui.selected_book >= ui.current_buffer_size - 1) {
                        ui.selected_book = 0;
                    } else {
                        ui.selected_book++;
                    }
                    draw_list(&ui, &store);
                } else {
                    if (ui.selected_book_attr == ANNOTATION_STR) {
                        ui.selected_book_attr = TITLE_STR;
                    } else {
                        ui.selected_book_attr++;
                    }
                    draw_book_info(&store.catalog[ui.selected_book], &ui, true);
                }
                break;
            case KEY_UP:
                if (ui.list_win_editing) {
                    if (ui.selected_book <= 0 || ui.selected_book >= ui.current_buffer_size) {
                        ui.selected_book = ui.current_buffer_size - 1;
                    } else {
                        ui.selected_book--;
                    }
                    draw_list(&ui, &store);
                } else {
                    if (ui.selected_book_attr == TITLE_STR) {
                        ui.selected_book_attr = ANNOTATION_STR;
                    } else {
                        ui.selected_book_attr--;
                    }
                    draw_book_info(&store.catalog[ui.selected_book], &ui, true);
                }
                break;
            case KEY_F(4):
                ui.search_fun = by_title;
                draw_list(&ui, &store);
                break;
            case KEY_F(5):
                ui.search_fun = by_author;
                draw_list(&ui, &store);
                break;
            case KEY_F(6):
                ui.search_fun = by_annotation;
                draw_list(&ui, &store);
                break;
            case KEY_F(7):
                ui.search_fun = by_tags;
                draw_list(&ui, &store);
                break;
            case KEY_F(8):
                if (!ui.list_win_editing){
                    sprintf(command, "change %d %s %s", ui.selected_book, get_attr_by_num(ui.selected_book_attr),
                            get_attr_value_by_num(ui.selected_book_attr, &ui.fields));
                    send(server_fd, command, strlen(command), 0);
                }
                break;
            case KEY_F(3):
                sprintf(command, "%s", ADD);
                send(server_fd, command, 6, 0);
                break;
            case KEY_F(1)/*'z'*/:
                sprintf(command, "%s %d", TAKE, ui.selected_book);
                send(server_fd, command, 8, 0);
                break;
            case KEY_F(2)/*'x'*/:
                sprintf(command, "%s %d", RETURN, ui.selected_book);
                send(server_fd, command, 10, 0);
                break;
            case KEY_BACKSPACE:
                if (ui.list_win_editing) {
                    if (ui.search_field_len > 0) {
                        ui.search_field[--ui.search_field_len] = '\0';
                        draw_search_field(&ui);
                    } else {
                        ui.search_fun = NULL;
                    }
                } else {
                    switch (ui.selected_book_attr) {
                        case 0:
                            ui.fields.title[--ui.fields.title_len] = '\0';
                            break;
                        case 1:
                            ui.fields.author[--ui.fields.author_len] = '\0';
                            break;
                        case 2:
                            ui.fields.tags[--ui.fields.tags_len] = '\0';
                            break;
                        case 3:
                            ui.fields.annotation[--ui.fields.annotation_len] = '\0';
                            break;
                    }
                    draw_book_info(ui.buffer[ui.selected_book], &ui, false);
                }
                break;
            case KEY_LEFT:
                if (!ui.list_win_editing) {
                    ui.list_win_editing = !ui.list_win_editing;
                    draw_book_info(ui.buffer[ui.selected_book], &ui, true);
                }
                break;
            case KEY_RIGHT:
                if (ui.list_win_editing) {
                    ui.list_win_editing = !ui.list_win_editing;
                    draw_book_info(ui.buffer[ui.selected_book], &ui, true);
                }
                break;
            case KEY_HOME:
            case KEY_END:
            case KEY_F(9):
            case KEY_F(10):
            case KEY_F(11):
                break;
            default:
                if (ui.list_win_editing) {
                    ui.search_field[ui.search_field_len++] = cur;
                    draw_search_field(&ui);
                } else {
                    switch (ui.selected_book_attr) {
                        case 0:
                            ui.fields.title[ui.fields.title_len++] = cur;
                            break;
                        case 1:
                            ui.fields.author[ui.fields.author_len++] = cur;
                            break;
                        case 2:
                            ui.fields.tags[ui.fields.tags_len++] = cur;
                            break;
                        case 3:
                            ui.fields.annotation[ui.fields.annotation_len++] = cur;
                            break;
                    }
                    draw_book_info(ui.buffer[ui.selected_book], &ui, false);
                }
                break;
        }
        refresh_all(&ui, false, true);
        usleep(100000);
        cur = getch();
    }
    send(server_fd, QUIT, 4, 0);
    endwin();
    close(server_fd);
    return 0;
}