#ifndef COMMANDHANDLER_HPP
#define COMMANDHANDLER_HPP

#include <string>
#include "Client.hpp"   // Incluez si nécessaire pour la gestion des clients
#include "Channel.hpp"  // Incluez si nécessaire pour la gestion des canaux

class CommandHandler {
public:
    CommandHandler();

    void handleCommand(const std::string& command, Client& client, std::vector<Channel>& channels);

private:
    void handleNickCommand(const std::string& parameters, Client& client);
    void handleJoinCommand(const std::string& parameters, Client& client, std::vector<Channel>& channels);
    void handlePrivmsgCommand(const std::string& parameters, Client& client, std::vector<Channel>& channels);
    // Ajoutez d'autres méthodes pour gérer d'autres commandes IRC

    // Méthodes utilitaires si nécessaire
    std::string extractFirstParameter(const std::string& message);
};

#endif // COMMANDHANDLER_HPP
