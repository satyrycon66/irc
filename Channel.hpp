#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>
#include <iostream>
#include "Client.hpp"  // Incluez si nécessaire pour la gestion des utilisateurs dans le canal

class Channel {
public:
    Channel(const std::string& name);
    ~Channel();

    const std::string& getName() const;
    bool hasClient(const Client& client) const;
    void addClient(const Client& client);
    void removeClient(const Client& client);
    void broadcastMessage(const std::string& message, const Client& sender);
    const std::vector<Client>& getClients() const ;

private:
    std::string _name;
    std::vector<Client> _clients;

    // Ajoutez d'autres membres ou méthodes privées selon les besoins
};

#endif // CHANNEL_HPP
