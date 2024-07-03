#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <vector>
#include "Client.hpp"
class Channel;
class Client {
public:
    Client(int socket,int index);
    ~Client();
    // Client(const std::string& username);
    void setSocket(int socket);
    int getSocket() const;
    int getIndex() const;
    const std::string& getUsername() const;
    const std::string& getNick() const;
    bool isAuthenticated() const;
    std::vector<std::string> getAllModes() const;
    void setNick(const std::string& nick);
    void setUsername(const std::string& username);
    void authenticate();
    void addInvintedChannel(Channel *channel);
    bool isInvitedChannel(Channel *channel) const;
    bool isInChannel(const std::string& channelName) const;
    void setMode(const std::string& modes);
    bool hasMode(char mode) const ;
    void setUserMode(const std::string& nick, const std::string& mode, int fd);
    std::string getAllModesString() const;
    std::vector<Channel> getInvitedChannel() const;

    
      // Opérateur de comparaison ==
    bool operator==(const Client& other) const;

    // Opérateur de comparaison !=
    bool operator!=(const Client& other) const;

private:
    int _socket;
    std::string _username;
    std::string _nick;
    int _index;
    bool _authenticated;
    std::string _userModes; // nick / modes
    std::vector<Channel> _invitedChannels;

};
std::string toLower(const std::string& str);
std::string removeCRLF(const std::string& str);
#endif // CLIENT_HPP


// /server add IRC5 localhost/6667 -notls
// /set irc.server.IRC5.password secret
// /set irc.server.IRC5.nicks sim
// /set irc.server.IRC5.username simon
// /connect IRC5

/*
/server add IRC5 localhost/6667 -notls
/set irc.server.IRC5.password secret
/set irc.server.IRC5.nicks sim
/set irc.server.IRC5.username simon
/connect IRC5
*/