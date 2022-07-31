#include <iostream>
#include <string>

#include <cstring>

#include "tcp.hpp"

int main(int argc, char* argv[]) 
{
    char port[8];

    if (argc < 2)
        strncpy(port, "8000", 4);
    else 
        strncpy(port, argv[1], strlen(argv[1]));

    auto tcp = TcpServer(port);
    
    tcp.server_listen();

    return EXIT_SUCCESS;
}