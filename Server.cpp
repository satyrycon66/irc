#include "Server.hpp"

Server::Server(int port, const std::string& password)
    : _port(port), _password(password), _running(false)
{
    initSignals();
    _server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_server_fd == -1) {
        throw std::runtime_error("Failed to create socket");
    }

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(_port);

    while (bind(_server_fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
        std::cerr <<"Trying to Bind...\n";
        sleep(3);
    }

    fcntl(_server_fd, F_SETFL, O_NONBLOCK);

    if (listen(_server_fd, SOMAXCONN) < 0) {
        throw std::runtime_error("Listen failed");
    }

    for (int i = 0; i < MAX_CLIENTS; ++i) {
        _fds[i].fd = -1;
    }
    _fds[0].fd = _server_fd;
    _fds[0].events = POLLIN;
    _clients.push_back(Client(_server_fd));

}
Server* Server::serverInstance = nullptr;
Server::~Server()
{
    std::cout << "Closing Server...\n";
    close(_server_fd);
}
void Server::run()
{
    _running = true;
    std::cout << "Server started on port " << _port << std::endl;

    while (_running) {
        int poll_count = poll(_fds, MAX_CLIENTS, -1);

        if (poll_count == -1) {
            continue;
        }

        for (int i = 0; i < MAX_CLIENTS; ++i) {
            if (_fds[i].revents & POLLIN) {
                if (_fds[i].fd == _server_fd) {
                    acceptNewConnection();
                } else {
                    handleClientData(i);
                }
            }
        }
    }
}
void Server::acceptNewConnection() {
    int new_socket = accept(_server_fd, NULL, NULL);
    if (new_socket < 0) {
        std::cerr << "Accept failed: " << strerror(errno) << std::endl;
        return;
    }

    // Set the new socket to non-blocking mode
    if (fcntl(new_socket, F_SETFL, O_NONBLOCK) < 0) {
        std::cerr << "Failed to set non-blocking mode: " << strerror(errno) << std::endl;
        close(new_socket);
        return;
    }

    // Find an empty slot in the _fds array for the new client
    bool clientAdded = false;
    for (int i = 1; i < MAX_CLIENTS; ++i) {
        if (_fds[i].fd == -1) {

            _fds[i].fd = new_socket;
            _fds[i].events = POLLIN;
            _clients.push_back(Client(new_socket));
            std::cout << "New client connected: " << new_socket << std::endl;
            clientAdded = true;
            break;
        }
    }

    // If no empty slot is found, close the new socket
    if (!clientAdded) {
        std::cerr << "Maximum number of clients reached. Connection refused: " << new_socket << std::endl;
        close(new_socket);
    }
}
void Server::handleClientData(int client_index)
{
    if (std::string(buffer).find("\r\n") != std::string::npos ){
        tempBuffer.clear();
        bzero(buffer,1024);
    }
    int valread = read(_fds[client_index].fd, buffer, sizeof(buffer) - 1);
    if (std::string(buffer).find("\r\n") == std::string::npos  && valread) // if message doesnt end with /r/n
    {
        std::cout << "Incomplete message: " << buffer << ":\n";
        tempBuffer += (std::string(buffer));
        if (tempBuffer.find("\n") != std::string::npos)
            tempBuffer.erase(tempBuffer.find("\n"));
        std::cout << "temp: " << tempBuffer << std::endl;
    }
    else if (valread <= 0) {
        if (valread == 0) {
            std::cout << "Client disconnected: " << _fds[client_index].fd << std::endl;
        } else {
            std::cerr << "Read error from client: " << _fds[client_index].fd << std::endl;
        }

        close(_fds[client_index].fd);

        // Supprimer le client de la liste des clients
        for (std::vector<Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
            if (it->getSocket() == _fds[client_index].fd) {
                _clients.erase(it);
                break;
            }
        }
        _fds[client_index].fd = -1;
    } else {
        tempBuffer += std::string(buffer);
        tempBuffer.erase(tempBuffer.find("\r\n"));
        
        // std::cout << "tempBuffer :"+ tempBuffer + ":";
        std::cout << "Received from client " << _fds[client_index].fd << ": -:" << tempBuffer << ":-\n";
        tempBuffer += "\r\n";
        // Traiter les commandes IRC
        if ((strstr(tempBuffer.c_str(), "USER ") != nullptr) || (strstr(tempBuffer.c_str(), "NICK ") != nullptr)) {
            if (strstr(tempBuffer.c_str(), "USER ") != nullptr)
                handleUserCommand(client_index, tempBuffer.c_str());
            if (strstr(tempBuffer.c_str(), "NICK ") != nullptr) 
                handleNickCommand(client_index, tempBuffer.c_str());
        } else if (strncmp(tempBuffer.c_str(), "INVITE ", 7) == 0) {
            handleInviteCommand(tempBuffer.c_str(), client_index);
        } else if (strncmp(tempBuffer.c_str(), "PASS ", 5) == 0) {
            handlePassCommand(tempBuffer.c_str(), client_index);
        }else if (strncmp(tempBuffer.c_str(), "TOPIC ", 6) == 0) {
            handleTopicCommand(tempBuffer.c_str(), client_index);
        }else if (strncmp(tempBuffer.c_str(), "CAP END ", 8) == 0) {
            std::string response = ":server CAP * END\r\n";
            send(_fds[client_index].fd, response.c_str(), response.length(), 0);
        }else if (strncmp(tempBuffer.c_str(), "CAP LS ", 7) == 0) {
            std::string response = ":server CAP * LS :multi-prefix\r\n";
        }else if (strncmp(tempBuffer.c_str(), "CAP REQ ", 8) == 0) {
            std::string response = ":server CAP * ACK :multi-prefix\r\n";
            send(_fds[client_index].fd, response.c_str(), response.length(), 0);
        }else if (strncmp(tempBuffer.c_str(), "PRIVMSG ", 8) == 0) {
            handlePrivMsgCommand(client_index, tempBuffer.c_str(),_clients[client_index]);
        }else if (strncmp(tempBuffer.c_str(), "JOIN ", 5) == 0) {
            handleJoinCommand(tempBuffer.c_str(), client_index);
        }else if (strncmp(tempBuffer.c_str(), "MODE ", 5) == 0) {
            handleModeCommand(tempBuffer.c_str(), client_index);
        }else if (strncmp(tempBuffer.c_str(), "PING ", 5) == 0) {
            std::string pongResponse = "PONG " + std::string(tempBuffer.c_str() + 5) + "\r\n";
            send(_fds[client_index].fd, pongResponse.c_str(), pongResponse.length(), 0);
        }else if (strncmp(tempBuffer.c_str(), "KICK ", 5) == 0) {
            handleKickCommand(tempBuffer.c_str(), client_index);        
        }else if (strncmp(tempBuffer.c_str(), "PART ", 5) == 0) {
            handlePartCommand(tempBuffer.c_str(), client_index);        
        }else {
            std::cout << "Unhandled message: " << tempBuffer.c_str();
        }
        tempBuffer.clear();
    }
}
void Server::handleModeCommand(const char* buffer, int client_index)
{
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
        // std::cout <<"channel :"<< channel << ":\n";
        // std::cout <<"modes :"<< modes << ":\n";
        // std::cout <<"param :"<< thirdParam << ":\n";
    if (modes == channel)
        modes.clear();
    if (thirdParam == channel)
        thirdParam.clear();
    if (!isValidMode(modes)) {
        modes.clear();
        thirdParam.clear();
        // sendErrorMessage(client_index,"Invalid mode entry, handling only 'o', 'i', 't', 'k', 'l' preceded by operator +/-");
        return;
    }
   
    if (channel[0] == '#')
    {
        handleModeChannelCommand(channel,modes,thirdParam,client_index);
        
    } else if (!modes.empty() ){
       Client *updateClient ;
       std::vector<Channel>::iterator it = _channels.begin();
       while (it != _channels.end()) {
            if (it->hasClientNick(channel)) {
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
            }   
            it++;
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
        std::cout << "Password accepted for client " << _fds[client_index].fd << std::endl;
        _clients[client_index].authenticate();
    } else {
        std::cout << "Password rejected for client " << _fds[client_index].fd << std::endl;

        // Password rejected, handle accordingly (e.g., disconnect client, send error message)
        sendErrorMessage(client_index, "Incorrect password");
        handleClientDisconnect( _clients[client_index]);
    }
}
void Server::handleNickCommand(int client_index, const char* buffer)
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
        sendWelcomeMessage(client_index);
    }
}
void Server::handleUserCommand(int client_index, const char* buffer)
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
        sendWelcomeMessage(client_index);
    }
}
void Server::handleClientDisconnect(Client& client)
{
    std::cout << "Client disconnected: " << client.getSocket() << std::endl;
    for (std::vector<Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (*it == client) {
            _clients.erase(it);
            close(it->getSocket());
            break;
        }
    }
}

void Server::createChannel(const std::string& name, const std::string& password) {

    std::cout << "Creating Channel: " << name << std::endl;

    Channel newChannel(name,password);
    _channels.push_back(newChannel);
    newChannel.setUserMax(-1);

    std::cout << "Channel created: " << name << std::endl;
}
void Server::joinChannel(const std::string& channelName,const std::string& password, const Client& client, int client_index)
{
    std::cout << "Trying to join channel: " << channelName << std::endl;
    Channel* channel = findChannel(channelName);
    if (channel) {
        if (channel->hasClient(client) )
        {
            sendErrorMessage(client_index, "443 ERR_USERONCHANNEL :User is already on channel");
            return;
        }
        if (password != channel->getPassword() && channel->getPassword() != "" ){
            // std::string msg = ":server " + errorCode +" "+ errorMessage +" "+ channelName + " :" + errorDescription + "\r\n";
            std::string msg = "475 ERR_BADCHANNELKEY :Cannot join channel "+ channelName +" (Incorrect password)";
            sendErrorMessage(client_index, msg);
            // send(_clients[client_index].getSocket(),msg.c_str(),msg.length(),0);
            return ;
        }
        if (channel->hasMode('i')) {
            if (!client.isInvitedChannel(channel)){
            std::string msg = "473 ERR_INVITEONLYCHAN :Cannot join channel "+ channelName +" (Channel is on Invite ONLY)";
            sendErrorMessage(client_index, msg);
            // handleClientDisconnect(_clients[client_index]);
            return ;
            }
        }
        if (channel->isFull()) {
            std::string msg = "471 ERR_CHANNELISFULL :Cannot join channel "+ channelName +" (Channel is full)";
            sendErrorMessage(client_index, msg);
            // handleClientDisconnect(_clients[client_index]);
            return ;
        }
        std::cout << "Channel found: " << channelName << std::endl;
        if(channel->isEmpty()){
            channel->addClient(client);
            channel->getOneClient(_clients[client_index].getNick())->setMode("+o");
        }if (!channel->hasClient(client))
            channel->addClient(client);
        std::cout << "Client " << client.getSocket() << " joined channel " << channelName << std::endl;
        // Construire le message JOIN
        std::string joinMessage = ":" + client.getNick() + "!" + client.getUsername() + "@localhost JOIN " + channelName + "\r\n";
        // Envoyer le message JOIN à tous les clients du canal
        std::vector<Client> channelClients = channel->getClients();
        for (std::vector<Client>::const_iterator it = channelClients.begin(); it != channelClients.end(); ++it) {
            std::cout << "Sending JOIN message to client: " << it->getSocket() << std::endl; // Message de débogage
            send(it->getSocket(), joinMessage.c_str(), joinMessage.length(), 0);
        }
        // Envoyer le message MODE au client qui a rejoint
        if(!channel->getMode().empty()){
           std::string modeMessage = ":server MODE " + channelName + " " + channel->getMode() + "\r\n";
            send(client.getSocket(), modeMessage.c_str(), modeMessage.length(), 0); 
        }
        // Constructing the NAMES message
        std::string namesMessage = ":server 353 " + client.getNick() + " = " + channelName + " :";

        // Append nicknames of users in the channel with modes
        for (std::vector<Client>::iterator it = channelClients.begin(); it != channelClients.end(); ++it) {
            if (it->hasMode('o')) {
                namesMessage += "@";
            } else if (it->hasMode('v')) {
                namesMessage += "+";
            } else {
                namesMessage += " ";
            }
            namesMessage += it->getNick() + " ";
        }

        namesMessage += "\r\n"; // Add IRC protocol end of message indicator
        send(client.getSocket(), namesMessage.c_str(), namesMessage.length(), 0);

        // Envoyer le message de fin NAMES au client qui a rejoint
        std::string endNamesMessage = ":server 366 " + client.getNick() + " " + channelName + " :End of /NAMES list.\r\n";
        send(client.getSocket(), endNamesMessage.c_str(), endNamesMessage.length(), 0);
        
        } else {
        std::cerr << "Channel not found: " << channelName << std::endl;
        send(_clients[client_index].getSocket(), "403 ERR_NOSUCHCHANNEL", 22, 0);
        }
}
void Server::leaveChannel(const std::string& channelName, const Client& client)
{
    Channel* channel = findChannel(channelName);
    if (channel) {
        if (channel->hasClient(client)) {
            std::string partMessage = ":" + client.getNick() + "!~" + client.getUsername() + "@localhost PART " + channelName + "\r\n";

            // Envoyer le message PART à tous les autres clients dans le canal
            std::vector<Client>::const_iterator it;
            for (it = channel->getClients().begin(); it != channel->getClients().end(); ++it) {
                const Client& chanClient = *it;
                if (chanClient.getSocket() != client.getSocket()) {
                    send(chanClient.getSocket(), partMessage.c_str(), partMessage.length(), 0);
                }
            }

            // Envoyer le message PART au client lui-même
            send(client.getSocket(), partMessage.c_str(), partMessage.length(), 0);

            // Supprimer le client du canal
            channel->removeClient(client);
            std::cout << "Client " << client.getSocket() << " left channel " << channelName << std::endl;
            // if (channel->isEmpty())
            //     removeChannel(*channel);
        } else {
            std::cerr << "Client " << client.getSocket() << " is not in channel " << channelName << std::endl;
        }
    
    } else {
        std::cerr << "Channel not found: " << channelName << std::endl;
    }
}
Channel* Server::findChannel(const std::string& channelName)
{
    for (std::vector<Channel>::iterator it = _channels.begin(); it != _channels.end(); ++it) {
        if (it->getName() == removeCRLF(channelName)) {
            return &(*it);
        }
    }
    return nullptr;
}
void Server::removeChannel(const Channel& channel) {
    std::vector<Channel>::iterator it = _channels.begin();
    while (it != _channels.end())
    {
        if (it->getName() == channel.getName()){
            _channels.erase(it);
            std::cout << "Removed channel " << channel.getName() << " from channels.\n";
            return ;
        }

    }
        std::cout << "Channel " << channel.getName() << " not found in _channels.\n";
    
}
void Server::handleModeChannelCommand(std::string channel, std::string modes, std::string thirdParam,int client_index)
{
    Channel* chan = findChannel(channel);
        if (chan) {
            if (!chan->getOneClient(_clients[client_index].getNick())){ 
                std::string errorMessage = ":server 404 " + _clients[client_index].getNick() + " " + chan->getName() + " :You're not in the channel\r\n";
                send(_clients[client_index].getSocket(), errorMessage.c_str(), errorMessage.length(), 0);
                return;
            }else if (!chan->getOneClient(_clients[client_index].getNick())->hasMode('o')) {
                std::string errorMessage = ":server 482 " + chan->getOneClient(_clients[client_index].getNick())->getNick() + " " + chan->getName() + " :You're not channel operator\r\n";
                send(_clients[client_index].getSocket(), errorMessage.c_str(), errorMessage.length(), 0);
                return ;
            }else if (modes[0] == '-' && modes[1] == 'l'){
                chan->removeMode(chan->getUserMax());
                chan->removeMode('l');
                chan->setUserMax(-1);
                std::string newlimit = ":" + _clients[client_index].getNick() + " MODE " + channel + " " + modes + "\r\n";
                send(_clients[client_index].getSocket(), newlimit.c_str(), newlimit.length(), 0);
                sendChannelMessage(channel,newlimit,_clients[client_index].getSocket(),_clients[client_index]);
                return;
            } else if (modes[0] == '+' && modes[1] == 'l' && !thirdParam.empty() && std::stoi(thirdParam) > 0 && isnumber(thirdParam[0]) < MAX_CLIENTS){
                chan->setMode(modes +" "+ thirdParam);
                chan->setUserMax(std::stoi(thirdParam));
                std::string newlimit = ":" + _clients[client_index].getNick() + " MODE " + channel + " " + modes +" "+ thirdParam + "\r\n";
                send(_clients[client_index].getSocket(), newlimit.c_str(), newlimit.length(), 0);
                sendChannelMessage(channel,newlimit,_clients[client_index].getSocket(),_clients[client_index]);
                return;
            } else if (modes[0] == '+' && modes[1] == 'k'){
                chan->setMode(modes +" "+ thirdParam);
                chan->setPassword(thirdParam);
                std::string newpass = ":" + _clients[client_index].getNick() + " MODE " + channel + " " + modes +" "+ thirdParam +  "\r\n";
                send(_clients[client_index].getSocket(), newpass.c_str(), newpass.length(), 0);
                sendChannelMessage(channel,newpass,_clients[client_index].getSocket(),_clients[client_index]);
                return;
            } else if (modes[0] == '-' && modes[1] == 'k'){
                chan->removeMode('k');
                chan->setPassword("");
                std::string newpass = ":" + _clients[client_index].getNick() + " MODE " + channel + " " + modes +  "\r\n";
                send(_clients[client_index].getSocket(), newpass.c_str(), newpass.length(), 0);
                sendChannelMessage(channel,newpass,_clients[client_index].getSocket(),_clients[client_index]);
                return;
            }else if (modes[0] == '+' ){
                chan->setMode(modes + " " + thirdParam);
                std::string newmode = ":" + _clients[client_index].getNick() + " MODE " + channel + " " + modes +" "+ thirdParam +  "\r\n";
                send(_clients[client_index].getSocket(), newmode.c_str(), newmode.length(), 0);
                sendChannelMessage(channel,newmode,_clients[client_index].getSocket(),_clients[client_index]);
                return;
            }else if (modes[0] == '-' ){
                for (size_t i = 1 ; i < modes.length(); i++)
                    chan->removeMode(modes[i]);
                std::string removedMode = ":" + _clients[client_index].getNick() + " MODE " + channel + " " + modes +" "+ thirdParam +  "\r\n";
                send(_clients[client_index].getSocket(), removedMode.c_str(), removedMode.length(), 0);
                sendChannelMessage(channel,removedMode,_clients[client_index].getSocket(),_clients[client_index]);
                return;
            }else{
                std::string response;
                if (thirdParam.length() > 0)
                    response = ":" + _clients[client_index].getNick() + " MODE " + channel + " " + modes + " " + thirdParam+ "\r\n";
                else
                    response = ":" + _clients[client_index].getNick() + " MODE " + channel + " " + modes + "\r\n";

            send(_clients[client_index].getSocket(), response.c_str(), response.length(), 0);
            sendChannelMessage(channel,response,_clients[client_index].getSocket(),_clients[client_index]);
            }
        } else {
            std::cerr << "Channel Not found: " << channel << std::endl;
            send(_clients[client_index].getSocket(), "403 ERR_NOSUCHCHANNEL", 22, 0);
        }
}

void Server::sendErrorMessage(int client_index, const std::string& message) {
    // Prepare the error message to send
    std::string errorResponse;
    if (message[0] != ':'){
        errorResponse = ":server " + message + "\r\n";
       
    }
    else{
        errorResponse = message;
    }

    // Send the error message back to the client
    int len = errorResponse.length();
    int bytes_sent = send(_fds[client_index].fd, errorResponse.c_str(), len, 0);

    if (bytes_sent != len) {
        std::cerr << "Error sending error message to client " << _fds[client_index].fd << std::endl;
        // Handle send error if necessary
    } else {
        std::cout << "Sent error message to client " << _fds[client_index].fd << ": " << errorResponse << std::endl;
    }
}
void Server::stopRunning()
{
    this->_running = false;
}
void Server::initSignals() {
        serverInstance = this;  // Set the global pointer to this instance
        signal(SIGINT, handleSignal);
        signal(SIGTERM, handleSignal);
        signal(SIGQUIT, handleSignal);
}
void Server::handleSignal(int signal) {
        if (serverInstance) {
            serverInstance->stopRunning();  // Access stopRunning through serverInstance
        }
}
Client *Server::getClient(const std::string& nick)
{
    for (std::vector<Client>::iterator it = _clients.begin();it != _clients.end(); it++)
    {
        if (it->getNick() == nick)
            return &(*it);
    }
    return nullptr;
}
bool Server::clientExists(const std::string& nick) {
        for (std::vector<Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
            if (it->getNick() == nick) {
                return true; // Return true if Client with nickname found
            }
        }
        return false; // Return false if Client with nickname not found
    }