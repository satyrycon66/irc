#include "Channel.hpp"

// Constructeur
Channel::Channel(const std::string& name) : _name(name) {return ; }

Channel::~Channel() { return ;}

const std::string& Channel::getName() const {    return _name;}
const std::string& Channel::getTopic() const {    return _topic;}
std::string Channel::getMode() { return _channelModes;}
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
void Channel::setMode(const std::string& mode) {
        if (mode.empty()) {
            return; // Handle empty mode string (optional)
        }

        char operation = mode[0];
        for (size_t i = 1; i < mode.size(); ++i) {
        char modeChar = mode[i];

        if (operation == '+') {
            // Add mode character if it doesn't already exist
            if (_channelModes.find(modeChar) == std::string::npos) {
                _channelModes += modeChar;
            }
        } else if (operation == '-') {
            // Remove mode character if it exists
            size_t pos = _channelModes.find(modeChar);
            if (pos != std::string::npos) {
                _channelModes.erase(pos, 1);
            }
        }
    }
    }

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
