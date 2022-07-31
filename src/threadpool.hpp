#pragma once

#include <queue>
#include <vector>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <memory.h>

#include "connection.hpp"

class Threadpool
{
public:
    Threadpool();

    void start_tp();
    void add_connection(std::unique_ptr<Connection>);
    bool busy();
    void stop_tp();

    ~Threadpool();
    
private:
    void tp_loop();

    bool terminate_pool = false;

    /* How many threads are in the pool. */
    unsigned int threadpool_size;

    /* The actual thread pool. */
    std::vector<std::thread> threads;

    std::condition_variable semaphore;

    std::mutex m_queue;

    /* The task queue storing the fd. */
    std::queue<std::unique_ptr<Connection>> t_queue;
};