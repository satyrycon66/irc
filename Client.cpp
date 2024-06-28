#include "Client.hpp"
#include "Channel.hpp"
#include "Server.hpp"

Client::Client(int socket): _socket(socket), _authenticated(false){ return;}
// Client::Client(const std::string& username) : _username(username), _authenticated(false) {return }
Client::~Client(){ return ;}

int Client::getSocket() const { return _socket;}
const std::string& Client::getNick() const {return _nick;}
const std::string& Client::getUsername() const{    return _username;}
bool Client::isAuthenticated() const{ return _authenticated;}

void Client::setUsername(const std::string& username){    _username = username;}
void Client::setNick(const std::string& nick){    _nick = nick;}
void Client::authenticate(){    _authenticated = true;}
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
        } else {
            std::cerr << "Invalid mode operation: " << operation << std::endl;
        }
    }

/*for permission verification*/
    std::string Client::getAllModesString() const {
        return _userModes;
    }

    // std::vector<std::string> Client::getAllModes() const {
    //     return _userModes;
    // }
// Opérateur de comparaison ==
bool Client::operator==(const Client& other) const {
    return this->_username == other._username; // Comparaison basée sur le nom d'utilisateur par exemple
}

// Opérateur de comparaison !=
bool Client::operator!=(const Client& other) const {
    return !(*this == other); // Utilisation de l'opérateur == pour définir !=
}




// /server add IRC5 localhost/6667 -notls
// /set irc.server.IRC5.password secret
// /set irc.server.IRC5.nicks sim
// /set irc.server.IRC5.username simon
// /connect IRC5

// /server add IRC localhost/6667 -notls
// /set irc.server.IRC.password asd
// /set irc.server.IRC.nicks al
// /set irc.server.IRC.username alexis
// /connect IRC