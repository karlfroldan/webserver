#include "connection.hpp"

#include <sstream>
#include <string>
#include <iostream>
#include <fstream>

#include <strings.h>
#include <cstdio>
#include <cstring>

#include <sys/socket.h>
#include <sys/types.h>

#include <unistd.h>

const int REQUEST_BUF_SIZE = 1024;

std::vector<std::string> 
tokenize_request(char request_string[], ssize_t r_len)
{
    /*
    A GET REQUEST HAS THE FOLLOWING CONTENTS
    GET /webpage HTTP/1.1.

    We need to get the /webpage part and tokenize it.
    */

    std::vector<std::string> tokens;

    std::istringstream r_stream (request_string);
    std::string tok;

    int counter = 0;

    while (r_stream >> tok && counter++ < 3)
        tokens.push_back(tok);

    return tokens;
}

bool file_exists(std::string filename)
{
    if (FILE* fd = fopen(filename.c_str(), "r"))
    {
        fclose(fd);
        return true;
    }
    else
        return false;
}

Connection::Connection() {}
Connection::Connection(int fd) : conn_fd {fd} {}

void Connection::run_connection()
{
    char recv_buf[REQUEST_BUF_SIZE];

    //std::cerr << "run (1)" << std::endl;

    ssize_t recv_size { recv(conn_fd, recv_buf, REQUEST_BUF_SIZE, 0) };

    //std::cerr << "run (2)" << std::endl;
    if (recv_size < 0)
    {
        is_valid_flag = false;
        /* Skip execution */
        close(conn_fd);
        return;
    }

    //std::cerr << "run (3)" << std::endl;

    request_tokens = tokenize_request(recv_buf, strlen(recv_buf));

    // for (auto r: request_tokens)
    //     std::cerr << "req (0): " << r << std::endl;

    /* This prevents some weird segmentation fault. */
    if (request_tokens.size() < 2)
    {
        is_valid_flag = false;
        /* Skip execution. */
        close(conn_fd);
        return;
    }

    /* Let's find out which request type it is. */
    if (request_tokens[1] == "/")
        filename = "/index.html";
    else 
        filename = request_tokens[1];

    
    if (strstr(filename.c_str(), ".htm"))
        request_type = "text/html";
    else if (strstr(filename.c_str(), ".xhtml"))
        request_type = "text/xhtml";
    else if (strstr(filename.c_str(), ".png"))
        request_type = "image/png";
    else if (strstr(filename.c_str(), ".jpg") || strstr(filename.c_str(), "jpeg"))
        request_type = "image/jpeg";
    else if (strstr(filename.c_str(), ".css"))
        request_type = "text/css";
    else if (strstr(filename.c_str(), ".js"))
        request_type = "text/js";
    else if (strstr(filename.c_str(), ".ico"))
        request_type = "image/x-icon";
    else 
    {
        std::cerr << "[ERROR] Request type is invalid\n";
        is_valid_flag = false;
        /* Skip execution */
        send_data();
        return;
    }

    //std::cerr << "run (5)" << std::endl;

    is_valid_flag = true;
    send_data();

    //std::cerr << "run (6)" << std::endl;
}

void Connection::send_data()
{
    if (request_tokens[0] == "GET" && request_tokens[2] == "HTTP/1.1")
    {
        /* Create the full file path. */
        std::ostringstream string_builder;

        /* Build our full path to the html file */
        string_builder << "html" << filename;

        auto file_loc {string_builder.str()};
        bool is_binary { !strstr(request_type.c_str(), "text/") };

        if (file_exists(file_loc))
        {
            /* Open the file */
            std::ifstream nfile;

            if (!is_binary)
                nfile.open(file_loc);
            else 
                nfile.open(file_loc, std::ios::in | std::ios::binary);
            /* Find the file size. */
            nfile.seekg(std::ios::beg, std::ios::end);
            file_size = nfile.tellg();

            std::string file_buffer(file_size, ' ');
            nfile.seekg(std::ios::beg);
            nfile.read(&file_buffer[0], file_size);

            std::string status("HTTP/1.1 200 OK");

            /* Create the entire message */
            std::ostringstream message;
            message << "HTTP/1.1 200 OK\r\nContent-Length: " << file_size << "\r\n\r\n";
            /* Find out how many characters are in the header. */
            size_t header_length {(size_t) message.tellp()};

            message << file_buffer; 
            std::string message_str {message.str()};

            const char* buf {message_str.c_str()};
            const size_t buf_len {file_buffer.size() + header_length};

            /* Write the HTML page. */
            int bytes_sent = 0, bytes_n;

            const char* buf_ptr = buf;

            while (bytes_sent < buf_len)
            {
                bytes_n = send(conn_fd, (void*) buf, buf_len, 0);
                // advance.
                buf_ptr += bytes_n;
                bytes_sent += bytes_n;
            } 
            
            //std::cerr << "GET HTTP/1.1 " << filename << " bytes: " << bytes_sent << '\n';
        }
        else /* Send an error 404. */
        {
            if (!is_binary)
                send_404();
        }
            
    }
    else if (!is_valid_flag)
        send_404();

    close(conn_fd);
}

Connection::~Connection() 
{}

bool Connection::is_conn_valid()
{
    return is_valid_flag;
}

void Connection::send_404()
{
    std::string err_404 {"HTTP/1.1 404 NOT FOUND\r\nContent-Length: 13\r\n\r\n404 NOT FOUND"};


    if (send(conn_fd, (void*) err_404.c_str(), err_404.size(), 0) < 0)
    {
        std::cerr << "[ERROR] Error sending 404 message\n";
    }
        
}