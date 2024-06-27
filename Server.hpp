#ifndef SERVER_HPP
#define SERVER_HPP

#include "Client.hpp"
#include <string>
#include <vector>
#include <poll.h>
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <poll.h>
#include <vector>
#include <map>
#include <signal.h>
#include <sstream>

#define MAX_CLIENTS 100
#include "Channel.hpp"

class Server {
public:
    Server(int port, const std::string& password);
    ~Server();
    void run();
    void createChannel(const std::string& name);
    void joinChannel(const std::string& channelName, const Client& client);
    void leaveChannel(const std::string& channelName, const Client& client);
    void sendWelcomeMessage(int client_index);
    void sendChannelMessage(const std::string& channelName, const std::string& message, int senderSocket ,const Client& client);
    void sendErrorMessage(int client_index, const std::string& message);
    void stopRunning();
    void initSignals();

private:
    bool _running;
    static Server* serverInstance; 
    int _port;
    char buffer[1024];
    static void handleSignal(int signal);
    std::string _password;
    int _server_fd;
    struct pollfd _fds[MAX_CLIENTS];
    std::vector<Client> _clients;
    std::vector<Channel> _channels;
    struct sockaddr_in *_address;
        
    void acceptNewConnection();
    void handleClientData(int client_index);
    void handleInviteCommand(int client_index, const char* buffer);
    void handleJoinCommand(const char* buffer, int client_index) ;
    void handleKickCommand(const char* buffer, int client_index) ;
    void handlePartCommand(const char* buffer, int client_index) ;
    void handleTopicCommand(const char* buffer, int client_index) ;
    void handleInviteCommand(const char* buffer, int client_index) ;
    void handlePassCommand(const char *buffer, int client_index);
    void handleNickCommand(int client_index, const char* buffer);
    void handlePrivMsgCommand(int client_index, const char* buffer ,const Client& client);
    void handleModeCommand(const char* buffer, int client_index);
    void handleUserCommand(int client_index, const char* buffer);
    void handleClientDisconnect(Client& client);
    Channel* findChannel(const std::string& channelName);
};
std::string toLower(const std::string& str);
std::string removeCRLF(const std::string& str);
#endif // SERVER_HPP
