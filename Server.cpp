#include "Server.hpp"
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
#include <sstream>

#define MAX_CLIENTS 100

// Constructor
Server::Server(int port, const std::string& password)
    : _port(port), _password(password), _running(false)
{
    _server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_server_fd == -1) {
        throw std::runtime_error("Failed to create socket");
    }

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(_port);

    if (bind(_server_fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
        throw std::runtime_error("Bind failed");
    }

    fcntl(_server_fd, F_SETFL, O_NONBLOCK);

    if (listen(_server_fd, SOMAXCONN) < 0) {
        throw std::runtime_error("Listen failed");
    }

    for (int i = 0; i < MAX_CLIENTS; ++i) {
        _fds[i].fd = -1;
    }
    _fds[0].fd = _server_fd;
    _fds[0].events = POLLIN;
}

// Destructor
Server::~Server()
{
    close(_server_fd);
}

// Method to start the server
void Server::run()
{
    _running = true;
    std::cout << "Server started on port " << _port << std::endl;

    while (_running) {
        int poll_count = poll(_fds, MAX_CLIENTS, -1);

        if (poll_count == -1) {
            std::cerr << "Poll failed" << std::endl;
            continue;
        }

        for (int i = 0; i < MAX_CLIENTS; ++i) {
            if (_fds[i].revents & POLLIN) {
                if (_fds[i].fd == _server_fd) {
                    acceptNewConnection();
                } else {
                    handleClientData(i);
                }
            }
        }
    }
}

// Method to accept a new incoming connection
void Server::acceptNewConnection()
{
    int new_socket = accept(_server_fd, NULL, NULL);
    if (new_socket < 0) {
        std::cerr << "Accept failed" << std::endl;
        return;
    }

    fcntl(new_socket, F_SETFL, O_NONBLOCK);

    for (int i = 1; i < MAX_CLIENTS; ++i) {
        if (_fds[i].fd == -1) {
            _fds[i].fd = new_socket;
            _fds[i].events = POLLIN;
            _clients.push_back(Client(new_socket));
            std::cout << "New client connected: " << new_socket << std::endl;
            break;
        }
    }
}

// Method to handle data received from a client
void Server::handleClientData(int client_index)
{
    char buffer[1024];
    int valread = read(_fds[client_index].fd, buffer, sizeof(buffer) - 1);

    if (valread <= 0) {
        if (valread == 0) {
            std::cout << "Client disconnected: " << _fds[client_index].fd << std::endl;
        } else {
            std::cerr << "Read error from client: " << _fds[client_index].fd << std::endl;
        }

        close(_fds[client_index].fd);
        _fds[client_index].fd = -1;

        // Supprimer le client de la liste des clients
        for (std::vector<Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
            if (it->getSocket() == _fds[client_index].fd) {
                _clients.erase(it);
                break;
            }
        }
    } else {
        buffer[valread] = '\0';
        std::cout << "Received from client " << _fds[client_index].fd << ": " << buffer;

        // Traiter les commandes IRC
        if (strncmp(buffer, "NICK ", 5) == 0) {
            handleNickCommand(client_index, buffer);
        } else if (strncmp(buffer, "USER ", 5) == 0) {
            handleUserCommand(client_index, buffer);
        } else if (strncmp(buffer, "CAP LS", 6) == 0) {
            std::string response = ":server CAP * LS :multi-prefix\r\n";
            send(_fds[client_index].fd, response.c_str(), response.length(), 0);
        }else if (strncmp(buffer, "PRIVMSG ", 8) == 0) {
            handlePrivMsgCommand(client_index, buffer);
        } else if (strncmp(buffer, "JOIN ", 5) == 0) {
            std::string command(buffer);
            std::istringstream iss(command.substr(5)); // Ignorer le préfixe /JOIN

            std::string channelName;
            iss >> channelName;

            if (!channelName.empty()) {
                if (!findChannel(channelName)) {
                    createChannel(channelName);
                    
                }
                joinChannel(channelName, _clients[client_index]);
            }
        } else if (strncmp(buffer, "MODE ", 5) == 0) {
            // Implémenter la gestion du mode du channel
        } else if (strncmp(buffer, "PING ", 5) == 0) {
            // Répondre au message PING du client
            std::string pongResponse = "PONG " + std::string(buffer + 5) + "\r\n";
            send(_fds[client_index].fd, pongResponse.c_str(), pongResponse.length(), 0);
        }else if (strncmp(buffer, "PART ", 5) == 0) {
    std::string command(buffer);
    std::istringstream iss(command.substr(5));

    std::string channelName;
    iss >> channelName;

    if (!channelName.empty()) {
        Channel* channel = findChannel(channelName);
        if (channel) {
            try {
                leaveChannel(channelName, _clients[client_index]);
                // Send response to WeeChat indicating success
                std::string response = ":server PART " + _clients[client_index].getNick() + " :" + channelName + " Leaving channel\r\n";
                send(_fds[client_index].fd, response.c_str(), response.length(), 0);
            } catch (const std::exception& e) {
                std::cerr << "Failed to leave channel " << channelName << ": " << e.what() << std::endl;
            }
        } else {
            std::cerr << "Channel not found: " << channelName << std::endl;
        }
    }
}else if (strncmp(buffer, "/privmsg ", 9) == 0) {
            // Implémenter la logique pour envoyer un message privé
        } else {
            // Par défaut, diffuser le message à tous les clients sauf l'expéditeur
            for (int i = 1; i < MAX_CLIENTS; ++i) {
                if (_fds[i].fd > 0 && i != client_index) {
                    send(_fds[i].fd, buffer, valread, 0);
                }
            }
        }
    }
}

void Server::handlePrivMsgCommand(int client_index, const char* buffer) {
    std::string command(buffer);
    
    // Find PRIVMSG command and extract target and message
    size_t pos = command.find("PRIVMSG ");
    if (pos == std::string::npos) {
        std::cerr << "Invalid PRIVMSG command format" << std::endl;
        return;
    }
    
    // Move the position past "PRIVMSG "
    pos += 8;
    
    // Find end of target (channel name)
    size_t end_pos = command.find_first_of(" :", pos);
    if (end_pos == std::string::npos) {
        std::cerr << "Invalid PRIVMSG command format" << std::endl;
        return;
    }
    
    std::string channelName = command.substr(pos, end_pos - pos);
    
    // Find the message content
    std::string message;
    pos = command.find(" :", end_pos);
    if (pos != std::string::npos) {
        message = command.substr(pos + 2); // Skip " :"
    } else {
        std::cerr << "Invalid PRIVMSG command format" << std::endl;
        return;
    }
    
    std::cout << "Received PRIVMSG command from client " << _fds[client_index].fd << " to channel: " << channelName << ", message: " << message << std::endl;
    
    // Check if channelName is valid (e.g., starts with # for IRC channels)
    if (channelName.empty() || channelName[0] != '#') {
        std::cerr << "Invalid channel name in PRIVMSG command" << std::endl;
        return;
    }
    
    // Example: Send the message to all clients in the channel
    sendChannelMessage(channelName, message, _fds[client_index].fd);
}
    
  
void Server::sendChannelMessage(const std::string& channelName, const std::string& message, int senderSocket) {
    // Find the channel in _channels map
    Channel* channel = findChannel(channelName);
    if (!channel) {
        std::cerr << "Channel not found: " << channelName << std::endl;
        return;
    }

    // Get the list of clients in the channel
    const std::vector<int> clientSockets = channel->getSockets();
const std::vector<Client> clients = channel->getClients();

// Send the message to all clients in the channel
std::vector<int>::const_iterator socketIt;
std::vector<Client>::const_iterator clientIt;

for (socketIt = clientSockets.begin(); socketIt != clientSockets.end(); ++socketIt) {
    int clientSocket = *socketIt;

    // Exclude the sender from receiving their own message
    if (clientSocket != senderSocket) {
        // Find the index of the client in the clients vector
        int clientIndex = clientSocket - 4; // Assuming 4 is the offset for the client socket in your example

        // Check bounds to avoid out-of-range access
        if (clientIndex >= 0 && clientIndex < clients.size()) {
            std::string messageToSend = ":" + clients[clientIndex].getNick() + " PRIVMSG " + channelName + " :" + message + "\r\n";
            send(clientSocket, messageToSend.c_str(), messageToSend.length(), 0);
        }
    }
}
}
void Server::handleNickCommand(int client_index, const char* buffer)
{
    std::string nick(buffer + 5);
    nick.erase(std::remove(nick.begin(), nick.end(), '\r'), nick.end());
    nick.erase(std::remove(nick.begin(), nick.end(), '\n'), nick.end());
    
    _clients[client_index].setNick(nick);

    // Check if both NICK and USER commands have been received
    if (!_clients[client_index].getNick().empty() && !_clients[client_index].getUsername().empty()) {
        sendWelcomeMessage(client_index);
    }
}

void Server::handleUserCommand(int client_index, const char* buffer)
{
    std::string user(buffer + 5);
    std::istringstream iss(user);
    std::string username;
    iss >> username;

    _clients[client_index].setUsername(username);

    // Check if both NICK and USER commands have been received
    if (!_clients[client_index].getNick().empty() && !_clients[client_index].getUsername().empty()) {
        sendWelcomeMessage(client_index);
    }
}

void Server::sendWelcomeMessage(int client_index)
{
    std::string welcome = ":server 001 " + _clients[client_index].getNick() + " :Welcome to the IRC server\r\n";
    send(_fds[client_index].fd, welcome.c_str(), welcome.length(), 0);
}

void Server::handleNewConnection(int socket)
{
    Client newClient(socket);
    _clients.push_back(newClient);
    std::cout << "New client connected: " << socket << std::endl;
}

void Server::handleClientData(Client& client, const std::string& data)
{
    std::cout << "Data received from client " << client.getSocket() << ": " << data << std::endl;
    // Handle the data received from the client
}

void Server::handleClientDisconnect(Client& client)
{
    std::cout << "Client disconnected: " << client.getSocket() << std::endl;
    for (std::vector<Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (*it == client) {
            _clients.erase(it);
            break;
        }
    }
}

void Server::createChannel(const std::string& name) {
    std::cout << "Creating Channel: " << name << std::endl;
    Channel newChannel(name);
    _channels.push_back(newChannel);

    std::cout << "Channel created: " << name << std::endl;

    // Construire et envoyer les messages JOIN et MODE à tous les clients connectés
    std::string joinMessage = ":" + std::string("server") + " JOIN " + name + "\r\n";
    std::string modeMessage = ":" + std::string("server") + " MODE " + name + " +nt\r\n";

    for (std::vector<Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        send(it->getSocket(), joinMessage.c_str(), joinMessage.length(), 0);
        send(it->getSocket(), modeMessage.c_str(), modeMessage.length(), 0);
    }
}


void Server::joinChannel(const std::string& channelName, const Client& client)
{
    std::cout << "Trying to join channel: " << channelName << std::endl;
    Channel* channel = findChannel(channelName);
    if (channel) {
        std::cout << "Channel found: " << channelName << std::endl;
        try {
            channel->addClient(client);
            std::cout << "Client " << client.getSocket() << " joined channel " << channelName << std::endl;

            // Construire le message JOIN
            std::string joinMessage = ":" + client.getNick() + " JOIN " + channelName + "\r\n";

            // Envoyer le message JOIN à tous les clients du canal
            const std::vector<Client> channelClients = channel->getClients();
            for (std::vector<Client>::const_iterator it = channelClients.begin(); it != channelClients.end(); ++it) {
                std::cout << "Sending JOIN message to client: " << it->getSocket() << std::endl; // Message de débogage
                send(it->getSocket(), joinMessage.c_str(), joinMessage.length(), 0);
            }

            // Envoyer le message MODE au client qui a rejoint
            std::string modeMessage = ":server MODE " + channelName + " +nt\r\n";
            send(client.getSocket(), modeMessage.c_str(), modeMessage.length(), 0);

            // Construire et envoyer le message NAMES au client qui a rejoint
            std::string namesMessage = ":server 353 " + client.getNick() + " = " + channelName + " :";
            for (std::vector<Client>::const_iterator it = channelClients.begin(); it != channelClients.end(); ++it) {
                namesMessage += it->getNick() + " ";
            }
            namesMessage += "\r\n";
            send(client.getSocket(), namesMessage.c_str(), namesMessage.length(), 0);

            // Envoyer le message de fin NAMES au client qui a rejoint
            std::string endNamesMessage = ":server 366 " + client.getNick() + " " + channelName + " :End of /NAMES list.\r\n";
            send(client.getSocket(), endNamesMessage.c_str(), endNamesMessage.length(), 0);

        } catch (const std::exception& e) {
            std::cerr << "Failed to add client to channel " << channelName << ": " << e.what() << std::endl;
        }
    } else {
        std::cerr << "Channel not found: " << channelName << std::endl;
    }
}




void Server::leaveChannel(const std::string& channelName, const Client& client)
{
    Channel* channel = findChannel(channelName);
    if (channel) {
        if (channel->hasClient(client)) {
            std::string partMessage = ":" + client.getNick() + "!~" + client.getUsername() + "@localhost PART " + channelName + "\r\n";

            // Envoyer le message PART à tous les autres clients dans le canal
            std::vector<Client>::const_iterator it;
            for (it = channel->getClients().begin(); it != channel->getClients().end(); ++it) {
                const Client& chanClient = *it;
                if (chanClient.getSocket() != client.getSocket()) {
                    send(chanClient.getSocket(), partMessage.c_str(), partMessage.length(), 0);
                }
            }

            // Envoyer le message PART au client lui-même
            send(client.getSocket(), partMessage.c_str(), partMessage.length(), 0);

            // Supprimer le client du canal
            channel->removeClient(client);
            std::cout << "Client " << client.getSocket() << " left channel " << channelName << std::endl;
        } else {
            std::cerr << "Client " << client.getSocket() << " is not in channel " << channelName << std::endl;
        }
    } else {
        std::cerr << "Channel not found: " << channelName << std::endl;
    }
}


Channel* Server::findChannel(const std::string& channelName)
{
    for (std::vector<Channel>::iterator it = _channels.begin(); it != _channels.end(); ++it) {
        if (it->getName() == channelName) {
            return &(*it);
        }
    }
    std::cerr << "Channel not found: " << channelName << std::endl;
    return NULL;
}
