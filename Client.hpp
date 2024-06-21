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
    bool isAuthenticated() const;
    void setUsername(const std::string& username);
    void authenticate();

      // Opérateur de comparaison ==
    bool operator==(const Client& other) const;

    // Opérateur de comparaison !=
    bool operator!=(const Client& other) const;

private:
    int _socket;
    std::string _username;
    bool _authenticated;
};

#endif // CLIENT_HPP
