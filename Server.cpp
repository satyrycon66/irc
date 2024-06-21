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
#include <sstream>

// Constructeur
Server::Server(int port, const std::string& password)
    : _port(port), _password(password), _running(false)
{
    // Création du socket
    _server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_server_fd == -1) {
        throw std::runtime_error("Failed to create socket");
    }

    // Configuration de l'adresse
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(_port);

    // Attachement du socket à l'adresse et au port spécifiés
    if (bind(_server_fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
        throw std::runtime_error("Bind failed");
    }

    // Passage du socket en mode non bloquant
    fcntl(_server_fd, F_SETFL, O_NONBLOCK);

    // Définition du socket en mode écoute (listen)
    if (listen(_server_fd, SOMAXCONN) < 0) {
        throw std::runtime_error("Listen failed");
    }

    // Initialisation de la structure pollfd pour le socket serveur
    _fds[0].fd = _server_fd;
    _fds[0].events = POLLIN;
}

// Destructeur
Server::~Server()
{
    // Fermeture du socket serveur
    close(_server_fd);
}

// Méthode pour démarrer le serveur
void Server::run()
{
    _running = true;
    std::cout << "Server started on port " << _port << std::endl;

    while (_running) {
        int poll_count = poll(_fds, MAX_CLIENTS, -1);  // Attend indéfiniment un événement

        if (poll_count == -1) {
            std::cerr << "Poll failed" << std::endl;
            continue;
        }

        // Vérification des événements pour chaque socket
        for (int i = 0; i < MAX_CLIENTS; ++i) {
            if (_fds[i].revents & POLLIN) {
                if (_fds[i].fd == _server_fd) {
                    // Nouvelle connexion entrante
                    acceptNewConnection();
                } else {
                    // Réception de données d'un client existant
                    handleClientData(i);
                }
            }
        }
    }
}

// Méthode pour accepter une nouvelle connexion entrante
void Server::acceptNewConnection()
{
    int new_socket = accept(_server_fd, nullptr, nullptr);
    if (new_socket < 0) {
        std::cerr << "Accept failed" << std::endl;
        return;
    }

    // Passage du nouveau socket en mode non bloquant
    fcntl(new_socket, F_SETFL, O_NONBLOCK);

    // Ajout du nouveau socket à la liste des fds à surveiller
    for (int i = 1; i < MAX_CLIENTS; ++i) {
        if (_fds[i].fd == 0) {
            _fds[i].fd = new_socket;
            _fds[i].events = POLLIN;
            std::cout << "New client connected: " << new_socket << std::endl;
            break;
        }
    }
}

void Server::handleClientData(int client_index) {
    char buffer[1024];
    int valread = read(_fds[client_index].fd, buffer, sizeof(buffer));

    if (valread <= 0) {
        // Erreur ou déconnexion du client
        if (valread == 0) {
            std::cout << "Client disconnected: " << _fds[client_index].fd << std::endl;
        } else {
            std::cerr << "Read error from client: " << _fds[client_index].fd << std::endl;
        }

        // Fermeture du socket client
        close(_fds[client_index].fd);
        _fds[client_index].fd = 0;

        // Supprimer le client de la liste _clients si nécessaire
        _clients.erase(_clients.begin() + client_index);
    } else {
        // Traitement des données reçues du client
        buffer[valread] = '\0';
        std::cout << "Received from client " << _fds[client_index].fd << ": " << buffer << std::endl;

        // Interpréter la commande IRC
             if (strncmp(buffer, "/join ", 6) == 0) {
            std::string command(buffer);
            std::istringstream iss(command.substr(6)); // Ignorer le préfixe /JOIN

            std::string channelName;
            iss >> channelName;

            if (!channelName.empty()) {
                // Vérifier si le premier caractère est un # pour créer un canal
                if (channelName[0] == '#') {
                    createChannel(channelName);
                    joinChannel(channelName, _clients[client_index]);
                } else {
                    joinChannel(channelName, _clients[client_index]);
                }
            }
        } else if (strncmp(buffer, "/PART ", 6) == 0) {
            // Implémenter la logique pour quitter un canal
        } else if (strncmp(buffer, "/PRIVMSG ", 9) == 0) {
            // Implémenter la logique pour envoyer un message privé
        } else {
            // Echo vers tous les autres clients connectés
            for (size_t i = 1; i < MAX_CLIENTS; ++i) {
            if (_fds[i].fd > 0 && i != static_cast<size_t>(client_index)) {
                send(_fds[i].fd, buffer, valread, 0);
                }
            }
        }
    }
}

void Server::handleNewConnection(int socket) {
    Client newClient(socket);
    _clients.push_back(newClient);
    std::cout << "New client connected: " << socket << std::endl;
}

void Server::handleClientData(Client& client, const std::string& data) {
    std::cout << "Data received from client " << client.getSocket() << ": " << data << std::endl;
    // Handle the data received from the client
}

void Server::handleClientDisconnect(Client& client) {
    std::cout << "Client disconnected: " << client.getSocket() << std::endl;
    std::vector<Client>::iterator it;
    for (it = _clients.begin(); it != _clients.end(); ++it) {
        if (*it == client) {
            _clients.erase(it);
            break;
        }
    }
}

void Server::createChannel(const std::string& name) {
    Channel newChannel(name);
    _channels.push_back(newChannel);
    std::cout << "Channel created: " << name << std::endl;
}

void Server::joinChannel(const std::string& channelName, const Client& client) {
    std::cout << "Trying to join channel: " << channelName << std::endl;
    Channel* channel = findChannel(channelName);
    if (channel) {
        std::cout << "Channel found: " << channelName << std::endl;
        try {
            channel->addClient(client);
            
            std::cout << "Client " << client.getSocket() << " joined channel " << channelName << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Failed to add client to channel " << channelName << ": " << e.what() << std::endl;
        }
    } else {
        std::cerr << "Channel not found: " << channelName << std::endl;
        // Optionally, throw an exception or return a status indicating failure
    }
}


void Server::leaveChannel(const std::string& channelName, const Client& client) {
    Channel* channel = findChannel(channelName);
    if (channel) {
        channel->removeClient(client);
        std::cout << "Client " << client.getSocket() << " left channel " << channelName << std::endl;
    } else {
        std::cerr << "Channel not found: " << channelName << std::endl;
    }
}

Channel* Server::findChannel(const std::string& channelName) {
    std::vector<Channel>::iterator it = _channels.begin();
    while (it != _channels.end()) {
        if (it->getName() == channelName) {
            return &(*it);
        }
        ++it;
    }
    std::cerr << "Channel not found: " << channelName << std::endl;
    return NULL;
}



