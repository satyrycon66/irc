#include "Channel.hpp"

// Constructeur
Channel::Channel(const std::string& name) : _name(name) {return ; }

Channel::~Channel() { return ;}

const std::string& Channel::getName() const {    return _name;}
const std::string& Channel::getTopic() const {    return _topic;}
const std::vector<Client>& Channel::getClients() const {    return this->_clients;}
std::vector<int> Channel::getSockets() const {
        std::vector<int> sockets;
        std::vector<Client>::const_iterator it;
        for (it = _clients.begin(); it != _clients.end(); ++it) {
            sockets.push_back(it->getSocket());}
        return sockets;
}

void Channel::setTopic(const std::string& topic) {    _topic = topic;}
void Channel::setName(const std::string& name) {    _name = name;}

// Méthode pour ajouter un client
void Channel::addClient(const Client& client) {   _clients.push_back(client);}

// Méthode pour retirer un client
void Channel::removeClient(const Client& client) {
    for (std::vector<Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (*it == client) {
            _clients.erase(it);
            break;
        }
    }
}

bool Channel::hasClient(const Client& client) const {
    for (std::vector<Client>::const_iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (*it == client)
            return true;    
    }
    return false;
}




// Autres méthodes de la classe Channel

