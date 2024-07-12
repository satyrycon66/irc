#include "Client.hpp"
#include "Channel.hpp"
#include "Server.hpp"

Client::Client(int socket,int index): _active(true),_socket(socket),_index(index), _authenticated(false),_isConnected(false){ 
   if (socket == 3){
        _nick = "Server";
        _username = "Server";
   } else{
    _nick = "";
    _username = "";
   }
    return;}
Client::~Client(){   return ;}


int Client::getSocket() const { return _socket;}
int Client::getIndex() const { return _index;}
const std::string& Client::getNick() const {return _nick;}
const std::string& Client::getUsername() const{    return _username;}
void Client::setAuth(int _bool) {_authenticated = _bool;}
void Client::setIndex(int index) {_index = index;}
void Client::decIndex() {_index--;std::cout << "New index: " << _index << std::endl;}
void Client::setSocket(int socket) {_socket = socket;}
void Client::setUsername(const std::string& username){    _username = username;}
void Client::setNick(const std::string& nick){    _nick = nick;}
void Client::authenticate(){    _authenticated = true;}
void Client::setActiveStatus(bool status){    _active = status;}
void Client::addInvintedChannel(Channel *channel){ _invitedChannels.push_back(*channel);}
void Client::clearInvChannels(){ _invitedChannels.clear();}
bool Client::hasIndex(int i) const {
    if (getIndex() == i)
        return true;
    else
        return false;}
bool Client::isInvitedChannel(Channel *channel) const{    
    for (std::vector<Channel>::const_iterator it = _invitedChannels.begin(); it != _invitedChannels.end(); it++){

    if (it->getName() == channel->getName())
        return true;
    }
    return false ;  
    }
bool Client::isAuthenticated() const{ return _authenticated;}
bool Client::isActive() const{ return _active;}

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
    if (this->_username == other._username && this->_nick == other._nick)
        return true;
    return false;
}
bool Client::operator!=(const Client& other) const {
    if (this->_username != other._username || this->_nick != other._nick)
        return true;
    return false;
}
void Client::setDisconnected(){ _isConnected = false;}
void Client::setConnected(){ _isConnected = true;}
bool Client::isConnected() const { return _isConnected;}

void Client::resetClient() {
    setNick("");
    setUsername("");
    setActiveStatus(false);
    setDisconnected();
    setAuth(false);
    clearInvChannels();
    _userModes.clear();
}

// /server add IRC5 localhost/6667 -notls
// /set irc.server.IRC5.password secret
// /set irc.server.IRC5.nicks sim
// /set irc.server.IRC5.username simon
// /connect IRC5
/*
/server add IRC localhost/6667 -notls
/set irc.server.IRC.password asd
/set irc.server.IRC.nicks al
/set irc.server.IRC.username alexis
/connect IRC
*/