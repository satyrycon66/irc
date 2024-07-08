#include "Channel.hpp"

Channel::Channel(const std::string& name,const std::string&password ) : _name(name), _password(password),_userLimit(-1) { std::cout << "Chanel :" << _name << "Created\n";return ; }

Channel::~Channel() { return ;}

const std::string& Channel::getName() const {    return _name;}
const std::string& Channel::getTopic() const {    return _topic;}
std::string Channel::getMode() { return _channelModes;}
const std::string& Channel::getPassword() const{ return _password;}
int Channel::getUserMax() const{ return _userLimit;}
std::vector<Client> Channel::getClients() const {    return this->_clients;}
std::vector<int> Channel::getSockets() const {
        std::vector<int> sockets;
        std::vector<Client>::const_iterator it;
        for (it = _clients.begin(); it != _clients.end(); ++it) {
            sockets.push_back(it->getSocket());}
        return sockets;
}
Client* Channel::getOneClient(const std::string& nick) {
    std::string searchedNick = removeCRLF(nick);
    for (std::vector<Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (toLower(it->getNick()) == searchedNick) 
            return &(*it);  // Return a pointer to the found client 
    }
        // std::cerr << "Client not Found: " << nick << "." <<std::endl;
        return NULL;  // Return NULL if client with given nick is not found
}

void Channel::setTopic(const std::string& topic)        {_topic = topic;}
void Channel::setName(const std::string& name)          {_name = name;}
void Channel::setPassword(const std::string& password)  {_password = password;}
void Channel::setUserMax(int max)                       {_userLimit = max;}

void Channel::setMode(const std::string& mode) {
        if (mode.empty()) {
            return; // Handle empty mode string (optional)
        }

        char operation = mode[0];
        for (size_t i = 1; i < mode.size() && !isspace(mode[i]) ; ++i) {
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
void Channel::removeMode(char mode) {
    size_t pos = _channelModes.find(mode);
    if (pos != std::string::npos) {
        _channelModes.erase(pos, 1);
    }
}

void Channel::addClient(const Client& client) {   _clients.push_back(client);}
void Channel::removeClientByName(const std::string& nickName) {
    for (std::vector<Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (it->getNick() == nickName) {
            _clients.erase(it);
            break;
        }
    }
}
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
bool Channel::hasClient(const Client& client) const {
    for (std::vector<Client>::const_iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (*it == client)
            return true;    
    }
    return false;
}
bool Channel::isEmpty() const {  return _clients.empty();}
bool Channel::hasClientNick(std::string nick) const {
    for (std::vector<Client>::const_iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (it->getNick() == nick)
            return true;    
    }
    return false;
}
bool Channel::isFull() const {
        if ((int)_clients.size() >= _userLimit && _userLimit > 0)
            return true;
        return false;
    }

bool Channel::operator==(const Channel& other) const {
    return this->_name == other._name; 
}
bool Channel::operator!=(const Channel& other) const {
    return this->_name != other._name; 
}