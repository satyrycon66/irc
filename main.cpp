#include "Server.hpp"
#include "Client.hpp"
#include <iostream>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
        return 1;
    }
    std::string portString = std::string(argv[1]);
    for (std::string::iterator it = portString.begin(); it != portString.end(); it++)
    {
        if (!std::isdigit(*it)){
           std::cerr << "Invalid Port, port must be numerical";
           return 1;
        }
    }
    std::string password = argv[2];
    int port = atoi(argv[1]);
    if (port < 6665 || port > 6669){
        std::cerr << "Invalid Port, supported ports are 6665 to 6669";
        return 1;
    }

    if (port){
        try {
            Server server(port, password);
            server.run();
        } catch (const std::exception &e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return 1; }
    }
    return 0;
}

// /server add IRC5 localhost/6667 -notls
// /set irc.server.IRC5.password secret
// /set irc.server.IRC5.nicks sim
// /set irc.server.IRC5.username simon
// /connect IRC5

// /server add IRC 10.18.200.40/6667 -notls
// /set irc.server.IRC.password asd
// /set irc.server.IRC.nicks al
// /set irc.server.IRC.username alexis
// /connect IRC