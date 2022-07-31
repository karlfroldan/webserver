cmake_minimum_required(VERSION 3.18)

project(WEBSERVER)

set(SRC src)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
set(CMAKE_CXX_STANDARD 17)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)


add_executable(webserver 
    ${SRC}/main.cpp
    ${SRC}/tcp.cpp
    ${SRC}/connection.cpp
    ${SRC}/threadpool.cpp
)

target_link_libraries(webserver PRIVATE Threads::Threads)