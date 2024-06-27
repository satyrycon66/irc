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


void Channel::setMode(const std::string& mode) {
    _channelModes = mode;
}

bool Channel::hasMode(char mode) const {
    return _channelModes.find(mode) != std::string::npos;
}

void Channel::removeMode(char mode) {
    size_t pos = _channelModes.find(mode);
    if (pos != std::string::npos) {
        _channelModes.erase(pos, 1);
    }
}

Client* Channel::getOneClient(const std::string& nick) {
        std::string temp = removeCRLF(nick);
        std::string lowerNick = toLower(temp);
        

        for (std::vector<Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
            if (toLower(it->getNick()) == lowerNick) {
                std::cout << "Client found: " << removeCRLF(nick) << std::endl;
                return &(*it);  // Return a pointer to the found client
            }
        }

        std::cerr << "Client not found: " << removeCRLF(nick)  << "." <<std::endl;
        return NULL;  // Return NULL if client with given nick is not found
    }

bool Channel::hasClient(const Client& client) const {
    for (std::vector<Client>::const_iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (*it == client)
            return true;    
    }
    return false;
}




// Autres méthodes de la classe Channel

