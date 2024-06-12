#include "Client.hpp"
#include <algorithm>

Client::Client(int socket, const std::string& username, const std::string& nickname)
    : socket(socket), username(username), nickname(nickname) {}

Client::~Client() {}

int Client::getSocket() const {
    return socket;
}

std::string Client::getUsername() const {
    return username;
}

std::string Client::getNickname() const {
    return nickname;
}

std::vector<std::string> Client::getChannels() const {
    return channels;
}

void Client::joinChannel(const std::string& channel) {
    channels.push_back(channel);
}

void Client::leaveChannel(const std::string& channel) {
    std::vector<std::string>::iterator it = std::find(channels.begin(), channels.end(), channel);
    if (it != channels.end()) {
        channels.erase(it);
    }
}

bool Client::isInChannel(const std::string& channel) const {
    std::vector<std::string>::const_iterator it = std::find(channels.begin(), channels.end(), channel);
    return it != channels.end();
}

// Dans ce fichier Client.cpp :

// Nous implémentons les méthodes déclarées dans Client.hpp pour accéder aux informations du client telles que le socket, le nom d'utilisateur, le surnom et les canaux, ainsi que des méthodes pour rejoindre, quitter et vérifier l'appartenance à un canal.
// Les méthodes joinChannel, leaveChannel et isInChannel sont implémentées pour gérer les opérations relatives aux canaux d'un client.
// Les autres méthodes sont implémentées pour accéder aux informations du client.
// Vous devrez compléter ce fichier Client.cpp en fonction des fonctionnalités spécifiques que vous souhaitez implémenter pour votre serveur IRC, comme la gestion des messages, les commandes IRC, etc.