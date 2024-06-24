#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class Client {
public:
    Client(int socket);
    ~Client();
    Client(const std::string& username);
    int getSocket() const;
    const std::string& getUsername() const;
    const std::string& getNick() const;
    bool isAuthenticated() const;
    void setNick(const std::string& nick);
    void setUsername(const std::string& username);
    void authenticate();
    bool isInChannel(const std::string& channelName) const;

      // Opérateur de comparaison ==
    bool operator==(const Client& other) const;

    // Opérateur de comparaison !=
    bool operator!=(const Client& other) const;

private:
    int _socket;
    std::string _username;
    std::string _nick;
    bool _authenticated;
};

#endif // CLIENT_HPP


// /server add IRC5 localhost/6667 -notls
// /set irc.server.IRC5.password secret
// /set irc.server.IRC5.nicks sim
// /set irc.server.IRC5.username simon
// /connect IRC5