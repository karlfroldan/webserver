#include "tcp.hpp"

#include <iostream>
#include <fstream>

#include <unistd.h>
#include <strings.h>

#include <stdlib.h>
#include <memory.h>
#include <unistd.h>

#include <string>
#include <sstream>

#include <utility>
#include <memory>

#include "connection.hpp"

const int BACKLOG  = 10;


/* Constructor. */
TcpServer::TcpServer(char* port) 
    : page_is_freed {false}
{
    this->sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (this->sockfd < 0)
    {
        std::cerr << "Socket creation error\n";
        exit(1);
    }

    // Clear the addres structure first.
    bzero((char*) &server_addr, sizeof(server_addr));

    this->portno = atoi(port);

    server_addr.sin_family = AF_INET; // Use IPv4
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(this->portno);

    // bind the socket to the current IP address.
    if (bind(this->sockfd, (sockaddr*) &server_addr, sizeof(sockaddr_in)) < 0)
    {
        perror("binding failed");
        exit(1);
    }

    this->clientlen = sizeof(sockaddr_in);

    std::cout << "Successfully binded to " << server_addr.sin_addr.s_addr << std::endl;
    std::cout << "Running on port: " << ntohs(server_addr.sin_port) << std::endl;

    // open the file.
    std::ifstream t("html/index.html");
    t.seekg(0, std::ios::end);
    file_size = t.tellg();

    std::string _file_buffer(file_size, ' ');
    file_buffer = _file_buffer;

    t.seekg(0);
    t.read(&file_buffer[0], file_size); 

    buf = file_buffer.c_str();
}



TcpServer::~TcpServer() 
{

    close(this->sockfd);
}

void TcpServer::server_listen() 
{
    if (listen(this->sockfd, BACKLOG) == -1)
    {
        perror("listening failed");
        exit(1);
    }

    tp.start_tp();

    // accept indefinetely with another thread.
    std::thread accept_thread([this] {
        while(true)
        {
            this->newclientfd = accept(this->sockfd, (sockaddr*) &client_addr, &clientlen);

            if (newclientfd == -1)
            {
                perror("accepting client failed");
                close(this->newclientfd);
            }
            else
                handle_connection(newclientfd);
        }
    });

    accept_thread.join();
    
}

void TcpServer::handle_connection(int fd)
{
    // Connection* conn = new Connection(fd);
    std::unique_ptr<Connection> conn_ptr { new Connection(fd) };
    tp.add_connection(std::move(conn_ptr));
    //conn.run_connection(fd, html_folder);
    // std::thread nthread(&Connection::run_connection, conn);
    //conn.run_connection(fd, html_folder);
    //nthread.detach();
}