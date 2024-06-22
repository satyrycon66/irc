// #include "CommandHandler.hpp"
// #include <iostream>  // À des fins de démonstration seulement

// CommandHandler::CommandHandler()
// {
//     // Initialisation si nécessaire
// }

// void CommandHandler::handleCommand(const std::string& command, Client& client, std::vector<Channel>& channels)
// {
//     // Exemple simplifié : traitement de quelques commandes IRC courantes
//     if (command.substr(0, 5) == "NICK ") {
//         std::string parameters = command.substr(5);
//         handleNickCommand(parameters, client);
//     } else if (command.substr(0, 5) == "JOIN ") {
//         std::string parameters = command.substr(5);
//         handleJoinCommand(parameters, client, channels);
//     } else if (command.substr(0, 7) == "PRIVMSG ") {
//         std::string parameters = command.substr(7);
//         handlePrivmsgCommand(parameters, client, channels);
//     } else {
//         // Gérer d'autres commandes IRC ici
//         std::cout << "Commande non reconnue ou non implémentée : " << command << std::endl;
//     }
// }

// void CommandHandler::handleNickCommand(const std::string& parameters, Client& client)
// {
//     // Implémentez le changement de pseudo du client
//     client.setUsername(parameters);
//     // À implémenter : envoyer une réponse au client, mise à jour des autres clients, etc.
// }

// void CommandHandler::handleJoinCommand(const std::string& parameters, Client& client, std::vector<Channel>& channels)
// {
//     (void)channels;
//     (void)client;
//     // Exemple simplifié : rejoindre un canal spécifié par le client
//     std::string channelName = extractFirstParameter(parameters);
//     // À implémenter : vérification du canal, ajout du client au canal, gestion des erreurs, etc.
// }

// void CommandHandler::handlePrivmsgCommand(const std::string& parameters, Client& client, std::vector<Channel>& channels)
// {
//     (void)channels;
//     (void)client;
//     // Exemple simplifié : envoyer un message privé ou à un canal
//     std::string firstParam = extractFirstParameter(parameters);
//     std::string message = parameters.substr(firstParam.size() + 1); // Le reste est le message

//     // À implémenter : vérification du destinataire, envoi du message aux bons clients/canaux, gestion des erreurs, etc.
// }

// std::string CommandHandler::extractFirstParameter(const std::string& message)
// {
//     // Extrait le premier paramètre d'un message
//     std::string::size_type pos = message.find(' ');
//     if (pos != std::string::npos) {
//         return message.substr(0, pos);
//     } else {
//         return message; // Si pas d'espace, retourne tout le message
//     }
// }
