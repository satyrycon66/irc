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
void Channel::broadcastMessage(const std::string& /* message */, const Client& sender) {
    // Utilisation d'un itérateur explicite
    for (std::vector<Client>::const_iterator it = _clients.begin(); it != _clients.end(); ++it) {
        // Diffusion du message à tous les clients sauf à l'expéditeur
        // Exemple imaginaire : if (it->getUsername() != sender.getUsername())
        if (*it != sender) {
            // Envoyer le message au client
            // Exemple imaginaire : it->sendMessage(message);
        }
    }
}

// Autres méthodes de la classe Channel

