cmake_minimum_required(VERSION 3.19)
project(syssoft3 C)

set(CMAKE_C_STANDARD 99)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(Curses REQUIRED)
add_library(bookstore SHARED bookstore/bookstore.c bookstore/bookstore.h client/ui/ui.c)
add_executable(server server/server.c server/server.h bookstore/commands.c bookstore/commands.h bookstore/bookstore.h bookstore/bookstore.c server/server_main.c responses.h)
add_executable(ui client/ui/ui.c client/ui/ui.h bookstore/commands.c bookstore/commands.h bookstore/bookstore.h bookstore/bookstore.c client/client.c client/client.h responses.h)
target_link_libraries(ui ${CURSES_LIBRARIES})
target_link_libraries(server Threads::Threads)
add_executable(client client/client.c client/client.h client/client_main.c responses.h bookstore/bookstore.c bookstore/bookstore.h bookstore/commands.h bookstore/commands.c)
target_link_libraries(client Threads::Threads)
target_link_libraries(ui Threads::Threads)
