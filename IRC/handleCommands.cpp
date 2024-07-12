#include "Server.hpp"

void Server::handleInviteCommand(const char* buffer, int client_index) {
    std::cout << "Handling " + std::string(buffer);
    if (!_clients[client_index].isConnected())
    {
        sendErrorMessage(client_index, "451 ERR_NOTREGISTERED :You have not registered");
        return;
    }
    std::string command(buffer);
    std::istringstream iss(command.substr(7)); // Ignorer le préfixe INVITE
    std::string nick;
    std::string channelName;
    iss >> channelName; // Récupérer le nom du canal
    iss >> nick; // Récupérer le nick à inviter
    if (channelName[0] != '#' && nick[0] != '#')
    {
        sendErrorMessage(client_index,  "Channel not found: " + channelName + ", Channels must start with '#'");
        std::cerr << "Channel not found: " << channelName << std::endl;
        return;
    }
    Channel *channel = findChannel(channelName);
    if (!channel)
    {
        channel = findChannel(nick);
        if (!channel)
        {
            sendErrorMessage(client_index,  "Channel not found: " + channelName);
            std::cerr << "Channel not found: " << channelName << std::endl;
            return;
        } else{
            std::string temp = channelName;
            channelName = nick;
            nick = temp;
        }
    }
    std::cout << "Test: nick = " << nick << " channel name = " <<channelName << "\n";
    if (!getClient(nick)->isConnected()){
        sendErrorMessage(client_index,"401 ERR_NOSUCHNICK :Client " + nick +" not found");
        std::cerr << "Error: Client not found" << std::endl;
        return ;
    }

    if (!nick.empty() && !channelName.empty() && clientExists(nick))  {
        // Vérifier les droits de l'utilisateur qui envoie la commande
        std::string userNick = _clients[client_index].getNick();
        std::string userMode = channel->getOneClient(_clients[client_index].getNick())->getAllModesString();
        // Suppose que 'o' (opérateur) est le mode nécessaire pour inviter
        if (userMode.find('o') != std::string::npos) {
            // Trouver le client invité dans la liste des clients
            Client* invitedClient = nullptr;
            for (std::vector<Client>::iterator it = _clients.begin(); it != _clients.end(); it++) {
                if (it->getNick() == nick && it->isConnected()) {
                    invitedClient = &(*it);
                    break;
                }
                if (it == _clients.end()){
                    sendErrorMessage(client_index,"401 ERR_NOSUCHNICK :Client " + nick +" not found");
                    return ;
                }
            }
            if (invitedClient->getNick() == nick && invitedClient->isConnected()) {
                invitedClient->addInvintedChannel(channel);
                std::string inviteMessage = ":"+ channel->getOneClient(_clients[client_index].getNick())->getNick() + " INVITE " + nick + " " + channelName + "\r\n";
                send(_fds[client_index].fd, inviteMessage.c_str(), inviteMessage.length(), 0);
                // Envoyer le message d'invitation au client invité
                std::string notifyMessage = ":" + _clients[client_index].getNick() + " INVITE " + nick + " :" + channelName + "\r\n";
                send(invitedClient->getSocket(), notifyMessage.c_str(), notifyMessage.length(), 0);
            } else {
                // Gestion de l'erreur : le client invité n'est pas trouvé
            std::cerr << "Error: Client to invite not found" << std::endl;
                }
        } else {
        // Gestion de l'erreur : l'utilisateur n'a pas les droits nécessaires
          std::string errorMessage = ":server 482 " + userNick + " " + channelName + " :You're not channel operator\r\n";
          send(_fds[client_index].fd, errorMessage.c_str(), errorMessage.length(), 0);
        }
    } else if (!clientExists(nick)) {
        sendErrorMessage(client_index,"401 ERR_NOSUCHNICK :Client " + nick +" not found");
        std::cerr << "Error: Client not found" << std::endl;    
    } else {
        std::cerr << "Error: Malformed INVITE command" << std::endl;
        
    }
}
void Server::handleJoinCommand(const char* buffer, int client_index) {
    if (!_clients[client_index].isConnected())
    {
        sendErrorMessage(client_index, "451 ERR_NOTREGISTERED :You have not registered");
        return;
    }
    std::string command(buffer);
    std::istringstream iss(command.substr(5)); // Skip "JOIN"

    // Variables to store parsed parts
    std::string channelName;
    std::string password;

    // Read channel name
    iss >> channelName;

    // Check if password is present
    std::string word;
    if (iss >> word) {
        password = word; // Password is present
    } else {
        password = ""; // Password is missing
    }
    if (channelName[0] != '#')
    {
        sendErrorMessage(client_index,  "Channel not found: " + channelName + ", Channels must start with '#'");
        std::cerr << "Channel not found: " << channelName << std::endl;
        return;
    }
    if (!channelName.empty()) {
        if (!findChannel(channelName)) 
            createChannel(channelName,password);            
        joinChannel(channelName,password, _clients[client_index], client_index);
    }
}
void Server::handlePrivMsgCommand( const char* buffer ,int client_index) {
    if (!_clients[client_index].isConnected())
    {
        sendErrorMessage(client_index, "451 ERR_NOTREGISTERED :You have not registered");
        return;
    }
    std::string command = removeCRLF(buffer);
    std::cout << "-Handling :" + command + ":\n";
    
    // Find PRIVMSG command and extract target and message
    size_t pos = command.find("PRIVMSG ");
    if (pos == std::string::npos) {
        std::cerr << "Invalid PRIVMSG command format" << std::endl;
        return;
    }
    // Move the position past "PRIVMSG "
    pos += 8;
    // Find end of target (channel name)
    size_t end_pos = command.find_first_of(" ", pos);
    if (end_pos == std::string::npos) {
        std::cerr << "Invalid PRIVMSG command format" << std::endl;
        return;
    }
    
    std::string channelName = command.substr(pos, end_pos - pos);
    // Find the message content
    std::string message;
    pos = command.find(" :", end_pos);
    if (pos != std::string::npos) 
        message = command.substr(pos + 2); // Skip " :"
    else {
        pos = command.find(" ", end_pos);
        if (pos != std::string::npos) 
            message = command.substr(pos + 1);
        else
            message = "";
    }
    
    if (channelName[0] != '#' && !channelName.empty() && clientExists(channelName)){
        std::string targetUser = channelName;
        Client* targetClient = nullptr;
        for (std::vector<Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
                if (it->getNick() == targetUser) {
                    targetClient = &(*it);
                    break;}
        }
        sendIRCPrivMessage(targetClient->getNick(),_clients[client_index].getNick(),removeCRLF(message));
    } else if (channelName.empty() || findChannel(channelName) == nullptr) {
        std::cerr << "Invalid channel name in PRIVMSG command" << std::endl;
    } else if (channelName[0] == '#'){
        Channel *channel_ref = findChannel(channelName);
        if (channel_ref)
        {
            if (!channel_ref->hasClientNick(_clients[client_index].getNick())) {
                std::string errorMessage = ":server 404 " + _clients[client_index].getNick() + " " + channel_ref->getName() + " :You're not in the channel\r\n";
                send(_clients[client_index].getSocket(), errorMessage.c_str(), errorMessage.length(), 0);
                std::cerr << "Client is not a member of channel " << channelName << std::endl;
                return;}
            else
                sendChannelMessage(channelName, message, _clients[client_index].getSocket(),_clients[client_index]);
        }
        else{
            sendErrorMessage(client_index, "403 ERR_NOSUCHCHANNEL :Channel " + channelName +" not found");
        }
    }
}
void Server::handlePartCommand(const char* buffer, int client_index) {
    if (!_clients[client_index].isConnected())
    {
        sendErrorMessage(client_index, "451 ERR_NOTREGISTERED :You have not registered");
        return;
    }
    std::cout << "Handling " + std::string(buffer);
    std::string command(removeCRLF(std::string(buffer)));
    std::istringstream iss(command.substr(5));

    std::string channelName;
    iss >> channelName;

    if (!channelName.empty() && channelName[0] == '#') {
        Channel* channel = findChannel(channelName);
        if (!channel)
            sendErrorMessage(client_index, "403 ERR_NOSUCHCHANNEL :Channel " + channelName +" not found");
        else if (!channel->hasClientNick(_clients[client_index].getNick()))
            sendErrorMessage(client_index,"401 ERR_NOSUCHNICK :Client " + _clients[client_index].getNick() +" not found");
        else if (channel) {
                leaveChannel(channelName, _clients[client_index],client_index);

                if (client_index == temp_index)
                {
                    tempBuffer.clear();
                    temp_index = -1;
                }
               } else {
               std::cerr << "Channel not found: " << channelName << std::endl;
        }
    }
}

void Server::handleTopicCommand(const char* buffer, int client_index) {
        if (!_clients[client_index].isConnected())
    {
        sendErrorMessage(client_index, "451 ERR_NOTREGISTERED :You have not registered");
        return;
    }
    std::cout << "Handling " + std::string(buffer);
    std::string command(buffer);
    std::istringstream iss(command.substr(6));

    std::string channelName;
    iss >> channelName;
    if (channelName[0] != '#')
    {
        sendErrorMessage(client_index,  "Channel not found: " + channelName + ", Channels must start with '#'");
        std::cerr << "Channel not found: " << channelName << std::endl;
        return;
    }
    Channel* channel = findChannel(channelName);
    if (channel) {
        if (channel->hasMode('t')){
            if ((!channel->getOneClient(_clients[client_index].getNick())->hasMode('o')) && command.find(" :")!= std::string::npos)
            {
                std::string errorMessage = ":server 482 " + _clients[client_index].getNick() + " " + channelName + " :You're not channel operator\r\n";
                send(_fds[client_index].fd, errorMessage.c_str(), errorMessage.length(), 0);
                return ;
            }
        }
        size_t pos = command.find(" :");
        if (pos != std::string::npos) {
            // L'utilisateur souhaite changer le sujet
            std::string userNick = _clients[client_index].getNick();
            std::string userMode = channel->getOneClient(_clients[client_index].getNick())->getAllModesString();
            // Suppose que 'o' (opérateur) est le mode nécessaire pour changer le sujet
            
                std::string topic = command.substr(pos + 2);
                channel->setTopic(topic);
                // Diffuser le nouveau sujet à tous les membres du canal
                std::string topicMessage = ":" + _clients[client_index].getNick() + " TOPIC " + channelName + " :" + topic + "\r\n";
                send(_clients[client_index].getSocket(), topicMessage.c_str(), topicMessage.length(), 0);
                sendChannelMessage(channelName, topicMessage, _clients[client_index].getSocket(), _clients[client_index]);
           
        } else {
            // L'utilisateur souhaite voir le sujet actuel
            std::string topicMessage = ":server 332 " + _clients[client_index].getNick() + " " + channelName + " :" + channel->getTopic() + "\r\n";
            send(_fds[client_index].fd, topicMessage.c_str(), topicMessage.length(), 0);
        }
    } else {
        std::cerr << "Channel not found: " << channelName << std::endl;
    }

}
void Server::handlePingCommand(const char* buffer, int client_index)
{
    std::cout << _clients[client_index].getNick() << " pinged\n";
    lastPingTimes[client_index] = time(nullptr);
    checkClientActivity();
    std::string pongResponse = "PONG " + std::string(buffer + 5) + "\r\n";
    send(_fds[client_index].fd, pongResponse.c_str(), pongResponse.length(), 0);
}
void Server::handleCAPCommand(const char* buffer, int client_index)
{
    std::cout << "-Handling " << buffer << std::endl;
        if ((strstr(buffer, "NICK ") != nullptr) || (strstr(buffer, "USER ")!= nullptr)) {
            handleNickCommand(buffer, client_index);
            handleUserCommand(buffer,client_index );
        }if (strncmp(buffer, "CAP END ", 8) == 0) {
            std::string response = ":server CAP * END\r\n";
            send(_fds[client_index].fd, response.c_str(), response.length(), 0);
        }else if (strncmp(buffer, "CAP LS ", 7) == 0) {
            std::string response = ":server CAP * LS :multi-prefix\r\n";
        }else if (strncmp(buffer, "CAP REQ ", 8) == 0) {
            std::string response = ":server CAP * ACK :multi-prefix\r\n";
            send(_fds[client_index].fd, response.c_str(), response.length(), 0);
        }
}
void Server::handleModeCommand(const char* buffer, int client_index)
{
    if (!_clients[client_index].isConnected())
    {
        sendErrorMessage(client_index, "461 ERR_NEEDMOREPARAMS :You are not connected");
        return;
    }
    std::string command(buffer);
    command = removeCRLF(command);
    size_t first_space_pos = command.find(' ');
    std::string mode_params = command.substr(first_space_pos + 1); // Get the part after "MODE "

    // Extract the parameters: channel, modes, thirdParam
    size_t second_space_pos = mode_params.find(' ');
    std::string channel;
    std::string modes;
    std::string thirdParam;

    if (second_space_pos != std::string::npos) {
        channel = (mode_params.substr(0, second_space_pos)); // Extract channel name

        size_t third_space_pos = mode_params.find(' ', second_space_pos + 1);
        if (third_space_pos != std::string::npos) {
            modes = removeCRLF(mode_params.substr(second_space_pos + 1, third_space_pos - second_space_pos - 1)); // Extract modes
            thirdParam = removeCRLF(mode_params.substr(third_space_pos + 1)); // Extract thirdParam
        } else {
            modes = removeCRLF(mode_params.substr(second_space_pos + 1)); // Extract modes if thirdParam not found
            thirdParam = "";
        }
    }
    if (modes == channel)
        modes.clear();
    if (thirdParam == channel)
        thirdParam.clear();
    if (!isValidMode(modes) && !modes.empty()) {
        modes.clear();
        thirdParam.clear();
        sendErrorMessage(client_index,"472 ERR_UNKNOWNMODE :Unknowned mode, handling only 'o', 'i', 't', 'k', 'l' preceded by operator +/-");
        return;
    }
   
    if (channel[0] == '#')
    {
        handleModeChannelCommand(channel,modes,thirdParam,client_index);
        
    } else if (!modes.empty() ){
       Client *updateClient ;
       Channel *channelRef = nullptr;
       std::vector<Channel>::iterator it = _channels.begin();
       while (it != _channels.end()) {
            if (it->hasClientNick(channel) && it->hasClientNick(_clients[client_index].getNick())) {
                updateClient = it->getOneClient(channel);
                
                if(!it->getOneClient(_clients[client_index].getNick())->hasMode('o') && modes.find('o') != std::string::npos)
                {
                    std::string errorMessage = ":server 482 " + it->getOneClient(_clients[client_index].getNick())->getNick() + " " + it->getName() + " :You're not channel operator\r\n";
                    send(_clients[client_index].getSocket(), errorMessage.c_str(), errorMessage.length(), 0);
                }
                else{

                updateClient->setMode(modes);
                std::string response = ":"+ it->getOneClient(_clients[client_index].getNick())->getNick() +" MODE " + it->getName() + " " + modes + " " + thirdParam + " "+ updateClient->getNick() + "\r\n";
                send(_clients[client_index].getSocket(), response.c_str(), response.length(), 0);
                sendChannelMessage(it->getName(),response,_clients[client_index].getSocket(),_clients[client_index]);
                }
                break ;
            }else if (it->hasClientNick(_clients[client_index].getNick()) && !it->hasClientNick(channel)) {
                channelRef = &(*it);
            }
            it++;
       }
        if (it ==_channels.end() && channelRef != nullptr)
        {
            std::string errorMessage;
            if (clientExists(channel))
              errorMessage = ":server 482 " + _clients[client_index].getNick() + " " + channelRef->getName() + " :" + channel + " not on channel" + channelRef->getName() + "\r\n";
            else
              errorMessage = ":server 482 " + _clients[client_index].getNick() + " " + channelRef->getName() + " :" + channel + " client doesnt exist\r\n";

              send(_clients[client_index].getSocket(), errorMessage.c_str(), errorMessage.length(), 0);
        } else if (channelRef == nullptr){
            std::string errorMessage = ":server 482 " + _clients[client_index].getNick() + " :You are not on a channel\r\n";
            sendErrorMessage(client_index,errorMessage);
        }
    }
}
void Server::handleKickCommand(const char* buffer, int client_index)
{
    std::string command(buffer);  // Convert buffer to std::string for easier manipulation

    // Find the first space to separate the command from the rest
    size_t first_space_pos = command.find(' ');
    if (first_space_pos == std::string::npos) {
        send(_clients[client_index].getSocket(), "400 Invalid command syntax", 25, 0);
        return;
    }
    std::string kick_params = command.substr(first_space_pos + 1); // Get the part after "KICK "
    
    // Find the second space to separate the channel name from the user
    size_t second_space_pos = kick_params.find(' ');
    if (second_space_pos == std::string::npos) {
        send(_clients[client_index].getSocket(), "400 Invalid command syntax", 25, 0);
        return;
    }

    std::string channel = kick_params.substr(0, second_space_pos); // Extract the channel name
    std::string target = kick_params.substr(second_space_pos + 1);

    Channel* chan = findChannel(channel);
    if (!chan) {
        send(_clients[client_index].getSocket(), "403 ERR_NOSUCHCHANNEL", 22, 0);
        return;
    }
    
    Client *kicker = chan->getOneClient(_clients[client_index].getNick());
    Client *kickTarget = chan->getOneClient(target);
    if (!kickTarget) {
        send(_clients[client_index].getSocket(), "403 ERR_NOSUCHNICK", 19, 0);
        return;
    }

    // Vérifier les droits de l'utilisateur qui envoie la commande
    std::string userNick = _clients[client_index].getNick();
    std::string userMode = kicker->getAllModesString();

    // Suppose que 'o' (opérateur) est le mode nécessaire pour kicker
    if (userMode.find('o') != std::string::npos) {
        std::string kickMessage = ":" + _clients[client_index].getNick() + "!~" + kickTarget->getUsername() + "@localhost KICK " + channel + " " + target + "\r\n";

        // Send KICK message to all clients in the channel except the one being kicked
        std::vector<Client> clients = chan->getClients();
        std::vector<Client>::const_iterator it;
        for (it = clients.begin(); it != clients.end(); ++it) {
            const Client& chanClient = *it;
            if (chanClient.getSocket() != kickTarget->getSocket()) {
                send(chanClient.getSocket(), kickMessage.c_str(), kickMessage.length(), 0);
            }
        }

        // Send KICK message to the client being kicked
        send(kickTarget->getSocket(), kickMessage.c_str(), kickMessage.length(), 0);

        // Remove the client from the channel
        chan->removeClient(*kickTarget);

        // Server console output
        std::cout << "Client " << kickTarget->getSocket() << " was kicked from channel " << channel << " by client: " << std::to_string(_clients[client_index].getSocket()) << std::endl;
    } else {
        // Gestion de l'erreur : l'utilisateur n'a pas les droits nécessaires
        std::string errorMessage = ":server 482 " + userNick + " " + channel + " :You're not channel operator\r\n";
        send(_clients[client_index].getSocket(), errorMessage.c_str(), errorMessage.length(), 0);
    }
}
void Server::handleNickCommand( const char* buffer, int client_index)
{
    std::cout << "-Handling " + std::string(buffer);
    if (!_clients[client_index].isAuthenticated())
    {
        sendErrorMessage(client_index, "451 :You have not registered");
        return;
    }
    // Find the start of the nickname after "NICK "
    const char* nick_start = strstr(buffer, "NICK ") + 5;
    
    // Extract nickname until the next space or end of string
    std::string nick;
    while (*nick_start && !isspace(*nick_start)) {
        nick += *nick_start++;
    }

    // Remove '\r' and '\n' characters from the nickname
    nick.erase(std::remove(nick.begin(), nick.end(), '\r'), nick.end());
    nick.erase(std::remove(nick.begin(), nick.end(), '\n'), nick.end());
    if (clientExists(nick))
    {
        std::string msg = "433 ERR_NICKNAMEINUSE";
        sendErrorMessage(client_index,msg);
        return;
    }
    _clients[client_index].setNick(nick);
    // Check if both NICK and USER commands have been received
    if (_clients[client_index].isAuthenticated() &&!_clients[client_index].getNick().empty() && !_clients[client_index].getUsername().empty()) {
        _clients[client_index].setConnected();
        sendWelcomeMessage(client_index);
    }
}
void Server::handleUserCommand(const char* buffer, int client_index)
{
    // Find the start of the username after "USER "
    std::cout << "-Handling " + std::string(buffer);
    if (!_clients[client_index].isAuthenticated())
    {
        sendErrorMessage(client_index, "451 :You have not registered");
        return;
    }
    const char* username_start = strstr(buffer, "USER ") + 5;
    
    // Extract username until the next space or end of string
    std::string username;
    while (*username_start && !isspace(*username_start)) {
        username += *username_start++;
    }
    _clients[client_index].setUsername(username);
    // Check if both NICK and USER commands have been received
    if (_clients[client_index].isAuthenticated() &&!_clients[client_index].getNick().empty() && !_clients[client_index].getUsername().empty()) {
        _clients[client_index].setConnected();
        sendWelcomeMessage(client_index);
    }
}
void Server::handleClientDisconnect(int client_index)
{
    const int index = client_index;
    int temp = _clients[client_index].getSocket();
    if (index == temp_index)
    {
        tempBuffer.clear();
        temp_index = -1;
    }
    if (_channels.size() > 0 && _clients[client_index].isConnected()){
    for (std::vector<Channel>::iterator it = _channels.begin(); it != _channels.end(); ++it){
        if (it->hasClientNick(_clients[client_index].getNick())) {
            leaveChannel(it->getName(),_clients[client_index], index);
            // break;
        }
    }
    }
    for (std::vector<Client>::iterator it = _clients.begin() + 1; it != _clients.end();it++) {

        if (it->getNick() != "" ? it->getNick() == _clients[client_index].getNick() : it->getIndex() == _clients[client_index].getIndex()) {
            std::cout << "Removing client: " << it->getIndex() << " nicknamed: " << it->getNick() << " on socket: "<< it->getSocket() << "\n";
            // close(it->getSocket());
            close(_fds[client_index].fd);
            _fds[client_index].fd = -1;
            _clients[client_index].resetClient();

            // _clients.erase(it);
            break ;
    }

}
// for (int i = 1; i < (int)_clients.size(); i++)
// {
//     _clients[i].setIndex(i);
// }
std::cout << "Client socket disconnected: " << temp << std::endl;
}
void Server::handlePassCommand(const char* buffer, int client_index)
{
    std::cout << "-Handling " << buffer;

    // Find the start of the password after "PASS "
    const char* pass_start = strstr(buffer, "PASS ") + 5;
    
    // Extract password until the next space or end of string
    std::string password;
    while (*pass_start && !isspace(*pass_start)) {
        password += *pass_start++;
    }

    // Remove '\r' and '\n' characters from the password
    password.erase(std::remove(password.begin(), password.end(), '\r'), password.end());
    password.erase(std::remove(password.begin(), password.end(), '\n'), password.end());

    // Compare the provided password with the stored password
    if (password == _password) {
        std::cout << "Password accepted for client " << client_index << std::endl;
        _clients[client_index].authenticate();
    } else {
        std::cout << "Password rejected for client " << client_index<< std::endl;

        // Password rejected, handle accordingly (e.g., disconnect client, send error message)
        sendErrorMessage(client_index, "Incorrect password");
    }
}
void Server::handleQuitCommand(const char* buffer, int client_index){
    (void)buffer;
    std::cout << "-Handling :QUIT socket:" << _clients[client_index].getSocket() <<"\n";
    handleClientDisconnect(client_index);

}

void Server::sendChannelMessage(const std::string& channelName, const std::string& message, int senderSocket ,const Client& client) {
    Channel* channel = findChannel(channelName);
    if (!channel) {
        std::cerr << "Channel not found: " << channelName << std::endl;
        return;
    }

    // Get the list of clients in the channel
    const std::vector<Client> channelClients = channel->getClients();

    // Send the message to all clients in the channel
    std::vector<Client>::const_iterator clientIt;

    for (clientIt = channelClients.begin(); clientIt != channelClients.end(); clientIt++) {
        int clientSocket = clientIt->getSocket();
        if (clientSocket != senderSocket) {
            if (clientIt->getIndex() > 0 ) {
                if (message[0] == ':'){
                    std::cout << "sending: " << message << "\n"; 
                    send(clientSocket, message.c_str(), message.length(), 0);
                }else{
                    std::string messageToSend = ":" + client.getNick() + " PRIVMSG " + channelName + " :" + message + "\r\n";
                    std::cout << "sending: " << messageToSend << "\n"; 
                    send(clientSocket, messageToSend.c_str(), messageToSend.length(), 0);
            }
        }
        }
    }
}
void Server::sendIRCPrivMessage(const std::string& targetNickname, const std::string& senderNickname, const std::string& message) {
    std::string replyMessage = ":" + targetNickname + " PRIVMSG " + senderNickname + " :" + message + "\r\n";
    
    int bytes_sent = send(getClient(targetNickname)->getSocket(),replyMessage.c_str(), replyMessage.length(), 0);
    if (bytes_sent == -1) {
        std::cerr << "Error: Failed to send message." << std::endl;
    } else if (bytes_sent != (int)replyMessage.length()) {
        std::cout << "Warning: " << bytes_sent << " bytes out of " << replyMessage.length() << " were sent" << std::endl;
    } 
}
std::string Server::sendIRCMessage(const std::string& targetNickname,const std::string& msgType, const std::string& senderNickname, const std::string& message) {
    std::string upperType = toUpper(msgType);
    std::string replyMessage = ":" + targetNickname + " " + upperType +" " + senderNickname + " :" + message + "\r\n";
    return replyMessage;
}
void Server::sendWelcomeMessage(int client_index)
{
    _clients[client_index].setConnected();
    std::string welcome = ":server 001 " + _clients[client_index].getNick() + " :Welcome to the IRC server\r\n";
    send(_fds[client_index].fd, welcome.c_str(), welcome.length(), 0);
}