#Version CLion
cmake_minimum_required(VERSION 3.29)
project(TP_IPC)

set(CMAKE_CXX_STANDARD 20)

add_executable(TP_IPC
        src/client.c
        #src/client_alternatif.c   - inutilisé finalement...
)

target_link_libraries(TP_IPC
        ${CMAKE_SOURCE_DIR}/lib/libseg.a)
#target_link_libraries(TP_IPC
#        ${CMAKE_SOURCE_DIR}/libseg.a
#)