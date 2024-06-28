#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <vector>
class Client {
public:
    Client(int socket);
    ~Client();
    // Client(const std::string& username);
    int getSocket() const;
    const std::string& getUsername() const;
    const std::string& getNick() const;
    bool isAuthenticated() const;
    std::vector<std::string> getAllModes() const;
    void setNick(const std::string& nick);
    void setUsername(const std::string& username);
    void authenticate();
    bool isInChannel(const std::string& channelName) const;
    void setMode(const std::string& modes);
    bool hasMode(char mode) const ;
    void setUserMode(const std::string& nick, const std::string& mode, int fd);

    // const std::map<std::string, std::string>&  getUserMode() const;
    std::string getAllModesString() const;
        // New methods for channel modes
    
      // Opérateur de comparaison ==
    bool operator==(const Client& other) const;

    // Opérateur de comparaison !=
    bool operator!=(const Client& other) const;

private:
    int _socket;
    std::string _username;
    std::string _nick;
    std::string _index;
    bool _authenticated;
    std::string _userModes; // nick / modes

};
std::string toLower(const std::string& str);
std::string removeCRLF(const std::string& str);
#endif // CLIENT_HPP


// /server add IRC5 localhost/6667 -notls
// /set irc.server.IRC5.password secret
// /set irc.server.IRC5.nicks sim
// /set irc.server.IRC5.username simon
// /connect IRC5