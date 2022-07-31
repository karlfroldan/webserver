#pragma once

#include <vector>
#include <string>

class Connection
{
public:
    Connection();
    Connection(int);
    ~Connection();

    void run_connection();

    bool is_conn_valid();
    void send_404();

    void send_data();

private:
    /* File descriptor of the client's connection. */
    int conn_fd;

    /* Type of the request. */
    std::string request_type;

    /* Request tokens */
    std::vector<std::string> request_tokens;

    /* Requested file. */
    std::string filename;

    /* html folder */
    std::string html_folder;

    /* file size */
    size_t file_size;

    /* connection valid flag.. */
    bool is_valid_flag;
};