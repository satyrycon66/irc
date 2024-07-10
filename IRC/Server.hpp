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
    void createChannel(const std::string& name, const std::string& password);
    void joinChannel(const std::string& channelName,const std::string& password, const Client& client,int client_index);
    void leaveChannel(const std::string& channelName, const Client& client, int client_index);
    void sendWelcomeMessage(int client_index);
    void sendChannelMessage(const std::string& channelName, const std::string& message, int senderSocket ,const Client& client);
    void sendErrorMessage(int client_index, const std::string& message);
    bool clientExists(const std::string& nick);
    Client *getClient(const std::string& nick);
    void stopRunning();
    void initSignals();
    int getClientsSize() const;
    void checkClientActivity();

private:
    std::map<int, time_t> lastPingTimes;
    int temp_index;
    bool _running;
    static Server* serverInstance; 
    int _port;
    char buffer[1024];
    std::string tempBuffer;
    std::string _password;
    int _server_fd;
    struct pollfd _fds[MAX_CLIENTS];
    std::vector<Client> _clients;
    std::vector<Channel> _channels;
    
    struct sockaddr_in *_address;
    typedef void (Server::*CommandHandler)(const char*, int);
    struct commandMap {
        const char* command;
        CommandHandler handler;
        };
    commandMap commandMap[13];
    void acceptNewConnection();
    static void handleSignal(int signal);
    void handleClientData(int client_index);
    void handleJoinCommand(const char* buffer, int client_index) ;
    void handleKickCommand(const char* buffer, int client_index) ;
    void handlePartCommand(const char* buffer, int client_index) ;
    void handleTopicCommand(const char* buffer, int client_index) ;
    void handleInviteCommand(const char* buffer, int client_index) ;
    void handlePassCommand(const char *buffer, int client_index);
    void handleNickCommand(const char* buffer, int client_index);
    void handlePrivMsgCommand( const char* buffer, int client_index);
    void handleModeCommand(const char* buffer, int client_index);
    void handleCAPCommand(const char* buffer, int client_index);
    void handlePingCommand(const char* buffer, int client_index);
    void handleQuitCommand(const char* buffer, int client_index);
    void handleUserCommand(const char* buffer, int client_index);
    void handleClientDisconnect(int client_index);
    void handleModeChannelCommand(std::string channel, std::string modes, std::string thirdParam,int client_index);
    Channel* findChannel(const std::string& channelName);
    void removeChannel(const Channel& channel);
    void sendIRCPrivMessage(const std::string& targetNickname, const std::string& senderNickname, const std::string& message);
    std::string sendIRCMessage(const std::string& targetNickname,const std::string& msgType, const std::string& senderNickname, const std::string& message);

    CommandHandler handlers[14];

};
std::string toLower(const std::string& str);
std::string toUpper(const std::string& str);
std::string removeCRLF(const std::string& str);
bool isValidMode(const std::string& modes);
bool isNumber(const std::string& s);
#endif // SERVER_HPP
