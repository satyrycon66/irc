#include "Server.hpp"

/*INVITE*/
void Server::handleInviteCommand(int client_index, const char* buffer) {
    std::cout << "Handling " + std::string(buffer);
    std::string command(buffer);
    std::istringstream iss(command.substr(8)); // Ignore "/invite "

    std::string nick;
    iss >> nick;

    std::string channelName;
    iss >> channelName;

    if (!nick.empty() && !channelName.empty()) {
        // Trouver le client destinataire
        bool found = false;
        for (size_t i = 0; i < _clients.size(); ++i) {
            if (_clients[i].getNick() == nick) {
                // Trouver ou créer le canal
                Channel* channel = findChannel(channelName);
                if (!channel) {
                    createChannel(channelName);
                    channel = findChannel(channelName);
                }
                if (channel) {
                    // Inviter le client au canal
                    joinChannel(channelName, _clients[i]);

                    // Envoyer un message de confirmation à l'inviteur
                    std::string response = ":server 341 " + _clients[client_index].getNick() + " " + _clients[i].getNick() + " :" + channelName + " Invitation sent\r\n";
                    send(_fds[client_index].fd, response.c_str(), response.length(), 0);

                    found = true;
                    break;
                }
            }
        }
        if (!found) {
            // Client non trouvé
            std::cerr << "Client not found: " << nick << std::endl;
        }
    }
}

/*INVITE*/
void Server::handleInviteCommand(const char* buffer, int client_index) {
    std::cout << "Handling " + std::string(buffer);
    std::string command(buffer);
    std::istringstream iss(command.substr(7)); // Ignorer le préfixe INVITE

    std::string nick;
    std::string channelName;
    iss >> nick; // Récupérer le nick à inviter
    iss >> channelName; // Récupérer le nom du canal

    if (!nick.empty() && !channelName.empty()) {
        // Trouver le client invité dans la liste des clients
        Client* invitedClient = nullptr;
        for (std::vector<Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
            if (it->getNick() == nick) {
                invitedClient = &(*it);
                break;
            }
        }

        if (invitedClient) {
            // Envoyer le message d'invitation au client qui a envoyé la commande
            std::string inviteMessage = ":server INVITE " + nick + " " + channelName + "\r\n";
            send(_fds[client_index].fd, inviteMessage.c_str(), inviteMessage.length(), 0);

            // Envoyer le message d'invitation au client invité
            std::string notifyMessage = ":" + _clients[client_index].getNick() + " INVITE " + nick + " :" + channelName + "\r\n";
            send(invitedClient->getSocket(), notifyMessage.c_str(), notifyMessage.length(), 0);
        } else {
            // Gestion de l'erreur : le client invité n'est pas trouvé
            std::cerr << "Error: Client to invite not found" << std::endl;
        }
    } else {
        // Gestion de l'erreur : la commande INVITE est malformée
        std::cerr << "Error: Malformed INVITE command" << std::endl;
    }
}

/*JOIN*/
void Server::handleJoinCommand(const char* buffer, int client_index) {
    std::string command(buffer);
    std::istringstream iss(command.substr(5)); // Ignorer le préfixe /JOIN

    std::string channelName;
    iss >> channelName;

    if (!channelName.empty()) {
        if (!findChannel(channelName)) {
            createChannel(channelName);
        }
        joinChannel(channelName, _clients[client_index]);
    }
}

/*PRIVMSG*/
void Server::handlePrivMsgCommand(int client_index, const char* buffer ,const Client& client) {
    std::string command(buffer);
    std::cout << "Handling " + std::string(buffer);
    
    // Find PRIVMSG command and extract target and message
    size_t pos = command.find("PRIVMSG ");
    if (pos == std::string::npos) {
        std::cerr << "Invalid PRIVMSG command format" << std::endl;
        return;
    }
    
    // Move the position past "PRIVMSG "
    pos += 8;
    
    // Find end of target (channel name)
    size_t end_pos = command.find_first_of(" :", pos);
    if (end_pos == std::string::npos) {
        std::cerr << "Invalid PRIVMSG command format" << std::endl;
        return;
    }
    
    std::string channelName = command.substr(pos, end_pos - pos);
    
    // Find the message content
    std::string message;
    pos = command.find(" :", end_pos);
    if (pos != std::string::npos) {
        message = command.substr(pos + 2); // Skip " :"
    } else {
        std::cerr << "Invalid PRIVMSG command format" << std::endl;
        return;
    }
    
    // Check if channelName is valid (e.g., starts with # for IRC channels)
    if (channelName.empty() || channelName[0] != '#') {
        std::cerr << "Invalid channel name in PRIVMSG command" << std::endl;
        return;
    }
    Channel *channel_ref = findChannel(channelName);
    if (channel_ref)
    {
        if (!channel_ref->hasClient(client)) {
            std::cerr << "Client is not a member of channel " << channelName << std::endl;
            return;}
        else
            sendChannelMessage(channelName, message, _fds[client_index].fd,client);
    }
}

/*PART*/
void Server::handlePartCommand(const char* buffer, int client_index) {
    std::cout << "Handling " + std::string(buffer);
    std::string command(buffer);
    std::istringstream iss(command.substr(5));

    std::string channelName;
    iss >> channelName;

    if (!channelName.empty()) {
        Channel* channel = findChannel(channelName);
        if (channel) {
            try {
                leaveChannel(channelName, _clients[client_index]);
                // Envoyer la réponse à WeeChat indiquant le succès
                std::string response = ":server PART " + _clients[client_index].getNick() + " :" + channelName + " Leaving channel\r\n";
                send(_fds[client_index].fd, response.c_str(), response.length(), 0);
            } catch (const std::exception& e) {
                std::cerr << "Failed to leave channel " << channelName << ": " << e.what() << std::endl;
            }
        } else {
            std::cerr << "Channel not found: " << channelName << std::endl;
        }
    }
}

/*TOPIC*/
void Server::handleTopicCommand(const char* buffer, int client_index) {
    std::cout << "Handling " + std::string(buffer);
    std::string command(buffer);
    std::istringstream iss(command.substr(6));

    std::string channelName;
    iss >> channelName;

    Channel* channel = findChannel(channelName);
    if (channel) {
        size_t pos = command.find(" :");
        if (pos != std::string::npos) {
            std::string topic = command.substr(pos + 2);
            channel->setTopic(topic);

            // Diffuser le nouveau sujet à tous les membres du canal
            std::string topicMessage = ":" + _clients[client_index].getNick() + " TOPIC " + channelName + " :" + topic + "\r\n";
            channel->broadcastMessage(topicMessage, _clients[client_index]);
        } else {
            // Envoyer le sujet actuel au client demandeur
            std::string topicMessage = ":server 332 " + _clients[client_index].getNick() + " " + channelName + " :" + channel->getTopic() + "\r\n";
            send(_fds[client_index].fd, topicMessage.c_str(), topicMessage.length(), 0);
        }
    } else {
        std::cerr << "Channel not found: " << channelName << std::endl;
    }
}

void Server::sendChannelMessage(const std::string& channelName, const std::string& message, int senderSocket ,const Client& client) {
    // Find the channel in _channels map
    Channel* channel = findChannel(channelName);
    if (!channel) {
        std::cerr << "Channel not found: " << channelName << std::endl;
        return;
    }

    // Get the list of clients in the channel
    const std::vector<int> clientSockets = channel->getSockets();
const std::vector<Client> clients = channel->getClients();

// Send the message to all clients in the channel
std::vector<int>::const_iterator socketIt;
std::vector<Client>::const_iterator clientIt;

for (socketIt = clientSockets.begin(); socketIt != clientSockets.end(); ++socketIt) {
    int clientSocket = *socketIt;
    
    // Exclude the sender from receiving their own message
    if (clientSocket != senderSocket) {
        // Find the index of the client in the clients vector
        int clientIndex = clientSocket - 4; // Assuming 4 is the offset for the client socket in your example

        // Check bounds to avoid out-of-range access
        if (clientIndex >= 0 && clientIndex < clients.size()) {
            std::string messageToSend = ":" + client.getNick() + " PRIVMSG " + channelName + " :" + message + "\r\n";
            send(clientSocket, messageToSend.c_str(), messageToSend.length(), 0);
        }
    }
}
}

void Server::sendWelcomeMessage(int client_index)
{
    std::string welcome = ":server 001 " + _clients[client_index].getNick() + " :Welcome to the IRC server\r\n";
    send(_fds[client_index].fd, welcome.c_str(), welcome.length(), 0);
}