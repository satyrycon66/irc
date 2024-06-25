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
    void handleNickCommand(int client_index, const char* buffer);
    void handlePrivMsgCommand(int client_index, const char* buffer ,const Client& client);
    void handleUserCommand(int client_index, const char* buffer);
    void sendWelcomeMessage(int client_index);
   void sendChannelMessage(const std::string& channelName, const std::string& message, int senderSocket ,const Client& client);
   void handleInviteCommand(int client_index, const char* buffer);
   void handleJoinCommand(const char* buffer, int client_index) ;
   void handlePartCommand(const char* buffer, int client_index) ;
   void handleTopicCommand(const char* buffer, int client_index) ;
   void handleInviteCommand(const char* buffer, int client_index) ;
private:
    int _port;
    std::string _password;
    bool _running;
    int _server_fd;
    struct pollfd _fds[MAX_CLIENTS];
    std::vector<Client> _clients;
    std::vector<Channel> _channels;
    struct sockaddr_in *_address;
    
    
    void acceptNewConnection();
    void handleClientData(int client_index);
    Channel* findChannel(const std::string& channelName);

   

    // Méthodes privées pour gérer les clients
   
   
    // Ajouter d'autres méthodes privées selon les besoins

};

#endif // SERVER_HPP
