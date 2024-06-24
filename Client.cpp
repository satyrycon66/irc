#include "Client.hpp"

Client::Client(int socket)
    : _socket(socket), _authenticated(false)
{
    // Initialisez d'autres membres au besoin
}

Client::~Client()
{
    // Nettoyage des ressources associées au client, si nécessaire
}

int Client::getSocket() const
{
    return _socket;
}

const std::string& Client::getNick() const {return _nick;}
const std::string& Client::getUsername() const{    return _username;}

bool Client::isAuthenticated() const
{
    return _authenticated;
}

void Client::setUsername(const std::string& username){    _username = username;}
void Client::setNick(const std::string& nick){    _nick = nick;}

void Client::authenticate()
{
    _authenticated = true;
}

Client::Client(const std::string& username) : _username(username) {
    // Initialisation éventuelle
}

// Implémentation de getUsername(), etc.

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