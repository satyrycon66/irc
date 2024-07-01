#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>
#include <iostream>
#include "Client.hpp"  // Incluez si nécessaire pour la gestion des utilisateurs dans le canal
class Client;
class Channel {
public:
    Channel(const std::string& name,const std::string&password );
    ~Channel();

    const std::string& getName() const;
    const std::string& getPassword() const;
    int getUserMax() const;
    bool hasClient(const Client& client) const;
    void addClient(const Client& client);
    void removeClient(const Client& client);
    void removeClientByName(const std::string& nickName);
    const std::vector<Client>& getClients() const ;
    std::vector<int> getSockets() const ;
    const std::string& getTopic() const;
    void setTopic(const std::string& topic);
    void setName(const std::string& name);
    void setPassword(const std::string& Password);
    void setUserMaxStr(const std::string& max);
    void setUserMax(int max);
    Client *getOneClient(const std::string& nick) ;
    void setMode(const std::string& mode);
    std::string  getMode();
    bool hasMode(char mode) const;
    void removeMode(char mode);
    bool hasClientNick(std::string nick) const ;
    bool isEmpty() const;
    bool isFull() const;
    bool operator==(const Channel& other) const;

    // Opérateur de comparaison !=
    bool operator!=(const Channel& other) const;
    

private:
    std::string _name;
    std::string _topic;
    std::vector<Client> _clients;
    std::string _channelModes;
    std::string _password;
    int _userLimit;
    // std::string _userLimitStr;

    // Private methods
    bool isValidChannelMode(char mode) const;
    bool isValidUserMode(char mode) const;
    // Ajoutez d'autres membres ou méthodes privées selon les besoins
};
std::string toLower(const std::string& str);
std::string removeCRLF(const std::string& str);
#endif // CHANNEL_HPP
