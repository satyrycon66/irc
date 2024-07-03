#include "Client.hpp"
#include "Channel.hpp"
#include "Server.hpp"

Client::Client(int socket,int index): _socket(socket),_index(index), _authenticated(false){ return;}
Client::~Client(){ return ;}

int Client::getSocket() const { return _socket;}
int Client::getIndex() const { return _index;}
const std::string& Client::getNick() const {return _nick;}
const std::string& Client::getUsername() const{    return _username;}

void Client::setUsername(const std::string& username){    _username = username;}
void Client::setNick(const std::string& nick){    _nick = nick;}
void Client::authenticate(){    _authenticated = true;}
void Client::addInvintedChannel(Channel *channel){ _invitedChannels.push_back(*channel);}

bool Client::isInvitedChannel(Channel *channel) const{    
    for (std::vector<Channel>::const_iterator it = _invitedChannels.begin(); it != _invitedChannels.end(); it++){

    if (it->getName() == channel->getName())
        return true;
    }
    return false ;  
    }
bool Client::isAuthenticated() const{ return _authenticated;}

bool Client::hasMode(char mode) const {
    std::string modeString(1, mode);
    return _userModes.find(modeString) != std::string::npos;
}
void Client::setMode(const std::string& modes) {
    if (modes.empty() || modes.size() < 2) {
        std::cerr << "Invalid mode string: " << modes << std::endl;
        return; // Handle invalid or empty mode string
    }
    std::string nick = getNick();
    char operation = modes[0];
    char modeChar = modes[1];
    if (modeChar == 'o'){
        if (operation == '+') {
            // Add mode character if it doesn't already exist
            if (_userModes.find(modeChar) == std::string::npos) {
                _userModes += modeChar;
                std::cout << "Mode '" << modeChar << "' added for client " << nick << std::endl;
            } else {
                std::cout << "Client " << nick << " already has mode '" << modeChar << "'" << std::endl;
            }
        } else if (operation == '-') {
            // Remove mode character if it exists
            size_t pos = _userModes.find(modeChar);
            if (pos != std::string::npos) {
                _userModes.erase(pos, 1);
                std::cout << "Mode '" << modeChar << "' removed for client " << nick << std::endl;
            } else {
                std::cout << "Client " << nick << " does not have mode '" << modeChar << "'" << std::endl;
            }
        }else {
            std::cerr << "Invalid mode operation: " << operation << std::endl;
        } 
    } else {
        std::cerr << "Invalid mode operation: " << operation << std::endl;
    }
}

std::string Client::getAllModesString() const {       return _userModes;}
std::vector<Channel> Client::getInvitedChannel() const{ return _invitedChannels;}
bool Client::operator==(const Client& other) const {
    return this->_username == other._username && this->_nick == other._nick; 
}
bool Client::operator!=(const Client& other) const {
    return this->_username != other._username || this->_nick != other._nick; 
    // return !(*this == other); // Utilisation de l'opérateur == pour définir !=
}




// /server add IRC5 localhost/6667 -notls
// /set irc.server.IRC5.password secret
// /set irc.server.IRC5.nicks sim
// /set irc.server.IRC5.username simon
// /connect IRC5
/*
/server add IRC 10.18.200.40/6667 -notls
/set irc.server.IRC.password asd
/set irc.server.IRC.nicks al
/set irc.server.IRC.username alexis
/connect IRC
*/