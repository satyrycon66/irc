#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>
#include <iostream>
#include "Client.hpp"  // Incluez si nécessaire pour la gestion des utilisateurs dans le canal
class Client;
class Channel {
public:
    Channel(const std::string& name);
    ~Channel();

    const std::string& getName() const;
    bool hasClient(const Client& client) const;
    void addClient(const Client& client);
    void removeClient(const Client& client);
    const std::vector<Client>& getClients() const ;
    std::vector<int> getSockets() const ;
    const std::string& getTopic() const;
    void setTopic(const std::string& topic);
    void setName(const std::string& name);
    Client *getOneClient(const std::string& nick) ;
    void setMode(const std::string& mode);
    bool hasMode(char mode) const;
    void removeMode(char mode);
    

private:
    std::string _name;
    std::string _topic;
    std::vector<Client> _clients;
    // Channel modes
    std::string _channelModes;

    // User modes
     // nick, mode

    // Private methods
    bool isValidChannelMode(char mode) const;
    bool isValidUserMode(char mode) const;
    // Ajoutez d'autres membres ou méthodes privées selon les besoins
};
std::string toLower(const std::string& str);
std::string removeCRLF(const std::string& str);
#endif // CHANNEL_HPP
