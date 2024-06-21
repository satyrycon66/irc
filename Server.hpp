#ifndef SERVER_HPP
#define SERVER_HPP

#include "Client.hpp"
#include <string>
#include <vector>
#include <poll.h>
#include "Channel.hpp"

const int MAX_CLIENTS = 100;
// Classe Server représentant le serveur IRC
class Server {
public:
    Server(int port, const std::string& password);
    ~Server();
    void run();
 void handleNewConnection(int socket);
    void handleClientData(Client& client, const std::string& data);
    void handleClientDisconnect(Client& client);
    void createChannel(const std::string& name);
    void joinChannel(const std::string& channelName, const Client& client);
    void leaveChannel(const std::string& channelName, const Client& client);

private:
    int _port;
    std::string _password;
    bool _running;
    int _server_fd;
    struct pollfd _fds[MAX_CLIENTS];
    std::vector<Client> _clients;
    std::vector<Channel> _channels;

    void acceptNewConnection();
    void handleClientData(int client_index);
    Channel* findChannel(const std::string& channelName);

   

    // Méthodes privées pour gérer les clients
   
   
    // Ajouter d'autres méthodes privées selon les besoins

};

#endif // SERVER_HPP
