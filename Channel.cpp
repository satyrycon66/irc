#include "Channel.hpp"

// Constructeur
Channel::Channel(const std::string& name) : _name(name) {
    // Initialisation éventuelle
}

Channel::~Channel() {
    // Code de destruction, si nécessaire
}

const std::string& Channel::getName() const {
    return _name;
}

// Méthode pour ajouter un client
void Channel::addClient(const Client& client) {
    _clients.push_back(client);
}

// Méthode pour retirer un client
void Channel::removeClient(const Client& client) {
    // Utilisation d'un itérateur explicite
    for (std::vector<Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        // Suppression du client en comparant les attributs pertinents (ex: nom d'utilisateur)
        // Exemple imaginaire : if (it->getUsername() == client.getUsername())
        if (*it == client) {
            _clients.erase(it);
            break;
        }
    }
}

// Méthode pour diffuser un message à tous les clients sauf l'expéditeur
void Channel::broadcastMessage(const std::string& message , const Client& sender) {
    // Utilisation d'un itérateur explicite
            std::cout << message + "***";
    for (std::vector<Client>::const_iterator it = _clients.begin(); it != _clients.end(); ++it) {
        // Diffusion du message à tous les clients sauf à l'expéditeur
        // Exemple imaginaire : if (it->getUsername() != sender.getUsername())

            std::cout << message + "***";
            // Exemple imaginaire : it->sendMessage(message);
        
    }
}

bool Channel::hasClient(const Client& client) const {
    // Implémentation de la méthode
    // Vérifie si le client est présent dans le vecteur _clients
    for (std::vector<Client>::const_iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (*it == client) {
            return true;
        }
    }
    return false;
}

std::vector<int> Channel::getSockets() const {
        // std::vector<Client> clients = getClients();
        std::vector<int> sockets;
        std::vector<Client>::const_iterator it;
        for (it = _clients.begin(); it != _clients.end(); ++it) {
            sockets.push_back(it->getSocket());
        }
        return sockets;
    }
const std::vector<Client>& Channel::getClients() const {
    return this->_clients;
}

// Autres méthodes de la classe Channel

