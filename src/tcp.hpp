#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <fstream>
#include <string>
#include <vector>
#include <thread>

#include "threadpool.hpp"

class TcpServer {

public: 
    TcpServer(char*);
    ~TcpServer();

    
    /* An infinite loop of the server just listening for requests. */
    void server_listen();

private:

    int sockfd;
    int newclientfd;
    int portno;
    socklen_t clientlen;
    
    // For accepting HTML file.
    size_t file_size;
    std::string file_buffer;
    
    std::string html_folder;
    

    sockaddr_in server_addr, client_addr;
    const char* buf;

    /* Contents of the page. */
    char* page;

    bool page_is_freed;

    /* The actual html file. The `html` folder will be prepended. */
    std::string html_file; 

    /**
     * Parses the Client's request. It should return a tokenized vector
     * `GET <page> HTTP1/1`
     * as the beginning of the request.
     */
    std::vector<std::string> parse_request(char[], ssize_t);

    /**
     * Handles the new connection. In a multithreaded web server,
     * this should create a new thread.
     */
    void handle_connection(int);

    /* Our threadpool */
    Threadpool tp;
};