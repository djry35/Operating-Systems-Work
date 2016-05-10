cmake_minimum_required(VERSION 2.8)

project(Threads)

set(CMAKE_C_FLAGS "-std=gnu99 -g")

find_library(dyn_array_lib dyn_array)

add_library(shellp src/shelp.c)

install(TARGETS shellp DESTINATION lib)

target_link_libraries(shellp ${dyn_array_lib})

install(FILES include/shelp.h DESTINATION include)

add_executable(shell src/shell.c)

target_link_libraries(shell shellp ${dyn_array_lib})

add_executable(tests test/test.c)

target_link_libraries(tests shellp ${dyn_array_lib})

