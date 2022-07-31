#include "threadpool.hpp"

#include <iostream>
#include <memory>
#include <utility>

Threadpool::Threadpool()
    : threadpool_size {std::thread::hardware_concurrency()}
{
    threads.resize(threadpool_size);
}

Threadpool::~Threadpool() 
{
    stop_tp();
}

void Threadpool::start_tp()
{
    for (auto i {0}; i < threadpool_size; ++i)
        threads[i] = std::thread(&Threadpool::tp_loop, this);
}

void Threadpool::tp_loop()
{
    while (true)
    {
        std::unique_ptr<Connection> conn_ptr;

        {
            std::unique_lock<std::mutex> lock(m_queue);

            semaphore.wait(lock, [this] {
                return !t_queue.empty() || terminate_pool;
            });

            if (terminate_pool)
                return;
            
            conn_ptr = std::move(t_queue.front());
            t_queue.pop();
        }

        conn_ptr->run_connection();
        // die.
        conn_ptr.~unique_ptr();
    }
}

void Threadpool::add_connection(std::unique_ptr<Connection> conn_ptr)
{
    {
        std::unique_lock<std::mutex> lock(m_queue);
        t_queue.push(std::move(conn_ptr));
    }

    semaphore.notify_one();
}

bool Threadpool::busy()
{
    bool pool_busy;
    {
        std::unique_lock<std::mutex> lock(m_queue);
        pool_busy = t_queue.empty();
    }

    return pool_busy;
}

void Threadpool::stop_tp()
{
    {
        std::unique_lock<std::mutex> lock(m_queue);
        terminate_pool = true;
    }

    semaphore.notify_all();

    for (std::thread& current_thread : threads)
        current_thread.join();

    threads.clear();
}
