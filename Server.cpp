#include "Server.hpp"

Server::Server(int port, const std::string& password)
    : _port(port), _password(password), _running(false)
{
    temp_index = -1;
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
    _clients.push_back(Client(_server_fd, 0));

    commandMap[0].command = "USER ";
    commandMap[0].handler = &Server::handleUserCommand;
    commandMap[1].command = "NICK ";
    commandMap[1].handler = &Server::handleNickCommand;
    commandMap[2].command = "INVITE ";
    commandMap[2].handler = &Server::handleInviteCommand;
    commandMap[3].command = "PASS ";
    commandMap[3].handler = &Server::handlePassCommand;
    commandMap[4].command = "TOPIC ";
    commandMap[4].handler = &Server::handleTopicCommand;
    commandMap[5].command = "JOIN ";
    commandMap[5].handler = &Server::handleJoinCommand;
    commandMap[6].command = "MODE ";
    commandMap[6].handler = &Server::handleModeCommand;
    commandMap[7].command = "PING ";
    commandMap[7].handler = &Server::handlePingCommand;
    commandMap[8].command = "KICK ";
    commandMap[8].handler = &Server::handleKickCommand;
    commandMap[9].command = "PRIVMSG ";
    commandMap[9].handler = &Server::handlePrivMsgCommand;
    commandMap[10].command = "CAP ";
    commandMap[10].handler = &Server::handleCAPCommand;
    commandMap[11].command = "PART ";
    commandMap[11].handler = &Server::handlePartCommand;
}
Server* Server::serverInstance = nullptr;
Server::~Server()
{
    std::cout << "Closing Server...\n";
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (_fds[i].fd > 0)
            close(_fds[i].fd );
    }
}
void Server::run()
{
    initSignals();
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
            _clients.push_back(Client(new_socket, i));
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

    bzero(buffer,1024);
    int valread = read(_fds[client_index].fd, buffer, sizeof(buffer) - 1);
    std::cout << "Recieving:" << removeCRLF(std::string(buffer)) << ":" << _fds[client_index].fd <<":\n";
      if (valread <= 0) {
        if (valread == 0) {
            handleClientDisconnect(_clients[client_index]);
            return ;
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
    } else if (std::string(buffer).find("\r\n") == std::string::npos  && valread) // if message doesnt end with /r/n
    {
        tempBuffer += (std::string(buffer));
        std::cout << "Incomplete message: " << buffer;
        temp_index = client_index;
        if (tempBuffer.find("\n") != std::string::npos)
            tempBuffer.erase(tempBuffer.find("\n"));
     }else {
        std::string data;
        if (temp_index == client_index && tempBuffer != std::string(buffer)){
            tempBuffer += (std::string(buffer));
            tempBuffer.erase(tempBuffer.find("\r\n"));
            tempBuffer += "\r\n";
            data += tempBuffer;
        }
        else{
            data += std::string(buffer);
            data.erase(data.find("\r\n"));
            data += "\r\n";
        }
        for (int i = 0; i < 12; i++) {
            if (strncmp(data.c_str(), commandMap[i].command, strlen(commandMap[i].command)) == 0) {
                (this->*commandMap[i].handler)(data.c_str(), client_index);
                break;
                }
            if (i == 11)
                std::cout << "Unhandled message: " << data; ///////////////////////////////////////////////////////////////////r
            }

        if (std::string(buffer).find("\r\n") != std::string::npos && temp_index == client_index){
        tempBuffer.clear();
        temp_index = -1;
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
        std::string joinMessage = ":" + _clients[client_index].getNick() + "!" + client.getUsername() + "@localhost JOIN " + channelName + "\r\n";
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
        std::string namesMessage = ":server 353 " + _clients[client_index].getNick() + " = " + channelName + " :";

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
        std::string endNamesMessage = ":server 366 " + _clients[client_index].getNick() + " " + channelName + " :End of /NAMES list.\r\n";
        send(client.getSocket(), endNamesMessage.c_str(), endNamesMessage.length(), 0);
        
        } else {
        std::cerr << "Channel not found: " << channelName << std::endl;
        send(_clients[client_index].getSocket(), "403 ERR_NOSUCHCHANNEL", 22, 0);
        }
}
void Server::leaveChannel(const std::string& channelName, const Client& clientz,int client_index)
{
    Channel* channel = findChannel(channelName);
    if (channel) {
        if (channel->hasClient(_clients[client_index])) {
            std::string partMessage = ":" + _clients[client_index].getNick() + "!~" + _clients[client_index].getUsername() + "@localhost PART " + channelName + "\r\n";

            // Envoyer le message PART à tous les autres clients dans le canal
            sendChannelMessage(channelName,partMessage,client_index,clientz);

            // Envoyer le message PART au client lui-même
            send(_clients[client_index].getSocket(), partMessage.c_str(), partMessage.length(), 0);

            // Supprimer le client du canal
            channel->removeClient(_clients[client_index]);
            std::cout << "Client " << _clients[client_index].getSocket() << " left channel " << channelName << std::endl;
            // if (channel->isEmpty())
            //     removeChannel(*channel);
        } else {
            std::cerr << "Client " << _clients[client_index].getSocket() << " is not in channel " << channelName << std::endl;
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