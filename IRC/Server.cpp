#include "Server.hpp"

Server::Server(int port, const std::string& password)
    : _running(false),_port(port) , _password(password)
{
    temp_index = -1;
    _server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_server_fd == -1) {
        throw std::runtime_error("Failed to create socket");
    }
    int optval = 1;
    setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval) < 0);
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
    commandMap[12].command = "QUIT ";
    commandMap[12].handler = &Server::handleQuitCommand;
}
Server* Server::serverInstance = nullptr;
Server::~Server()
{
    std::cout << "Closing Server...\n";
    for (int i = 1; i < MAX_CLIENTS; i++)
    {
        if (_fds[i].fd > 0)
            close(_fds[i].fd );
    }
    _channels.clear();
    _clients.clear();
    close(_server_fd);
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
            lastPingTimes[i] = time(nullptr);
            int j = 1;
            bool add = true;
            while( j < (int)_clients.size()) {

                if (_clients[j].hasIndex(i) && !_clients[j].isActive()){
                    _clients[j].setActiveStatus(true);
                    add = false;
                }
                j++;
            }
            if (add)
                _clients.push_back(Client(new_socket,i));
            std::cout << "New client " << i   << " connected on socket: " << new_socket << std::endl;
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
    int valread = recv(_fds[client_index].fd, buffer, sizeof(buffer), 0);
    std::cout << "Recieving : " << removeCRLF(std::string(buffer)) << "\n";
      if (valread <= 0) {
        if (valread == 0) {
            if (temp_index == client_index)
            {
                tempBuffer.clear();
                temp_index = -1;
            }
            handleQuitCommand(NULL,client_index);
            return ;
        } else {
            // std::cerr << "Read error from client: " << _fds[client_index].fd << std::endl;
            return ;
        // handleClientDisconnect(client_index);
        }
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
            data += tempBuffer;
        }
        else{
            data += std::string(buffer);
        }
        for (int i = 0; i < 13; i++) {
            if (strncmp(data.c_str(), commandMap[i].command, strlen(commandMap[i].command)) == 0) {
                (this->*commandMap[i].handler)(data.c_str(), client_index);
                break;
                }
            if (i == 13)
                std::cout << "Unhandled message: " << data; ///////////////////////////////////////////////////////////////////r
            }

        if (std::string(buffer).find("\r\n") != std::string::npos && temp_index == client_index){
        tempBuffer.clear();
        temp_index = -1;
        }
        checkClientActivity();
        printClients();
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
        for (std::vector<Client>::iterator it = channelClients.begin(); it != channelClients.end(); ++it) {
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
    (void)clientz;
    if (channelName[0] != '#')
    {
        sendErrorMessage(client_index,  "Channel not found: " + channelName + ", Channels must start with '#'");
        std::cerr << "Channel not found: " << channelName << std::endl;
        return;
    }
    Channel* channel = findChannel(channelName);
    if (channel) {
        if (channel->hasClientNick(_clients[client_index].getNick())) {
            std::string partMessage = ":" + _clients[client_index].getNick() + "!~" + _clients[client_index].getUsername() + "@localhost PART " + channelName + "\r\n";

            // Envoyer le message PART à tous les autres clients dans le canal
            sendChannelMessage(channelName,partMessage,client_index,_clients[client_index]);

            // Supprimer le client du canal
            channel->removeClientByName(_clients[client_index].getNick());
            std::cout << "Client " << _clients[client_index].getIndex() << " left channel " << channelName << std::endl;
        } else {
            std::cerr << "Client " << _clients[client_index].getIndex() << " is not in channel " << channelName << std::endl;
        }
    
    } else {
        std::cerr << "Channel not found: " << channelName << std::endl;
    }
}
Channel* Server::findChannel(const std::string& channelName)
{
    for (std::vector<Channel>::iterator it = _channels.begin(); it != _channels.end(); ) {
        if (it->getName() == removeCRLF(channelName)) {
            return &(*it);
        }
        it++;
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
        it++;
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
            } else if (modes[0] == '+' && modes[1] == 'l' && !thirdParam.empty() && isNumber(thirdParam) && (std::stoi(thirdParam) > 0 && std::stoi(thirdParam) < MAX_CLIENTS)){
                chan->setMode(modes +" "+ thirdParam);
                chan->setUserMax(std::stoi(thirdParam));
                std::string newlimit = ":" + _clients[client_index].getNick() + " MODE " + channel + " " + modes +" "+ thirdParam + "\r\n";
                send(_clients[client_index].getSocket(), newlimit.c_str(), newlimit.length(), 0);
                sendChannelMessage(channel,newlimit,_clients[client_index].getSocket(),_clients[client_index]);
                return;
            }else if (modes[0] == '+' ){
                chan->setMode(modes + " " + thirdParam);
                if (modes[1] == 'k')
                    chan->setPassword(thirdParam);
                std::string newmode = ":" + _clients[client_index].getNick() + " MODE " + channel + " " + modes +" "+ thirdParam +  "\r\n";
                send(_clients[client_index].getSocket(), newmode.c_str(), newmode.length(), 0);
                sendChannelMessage(channel,newmode,_clients[client_index].getSocket(),_clients[client_index]);
                return;
            }else if (modes[0] == '-' ){
                for (size_t i = 1 ; i < modes.length(); i++)
                    chan->removeMode(modes[i]);
                if (modes[1] == 'k'){
                chan->removeMode('k');
                chan->setPassword("");}
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
    (void) signal;
        if (serverInstance) {
            serverInstance->stopRunning();  // Access stopRunning through serverInstance
        }
}
Client *Server::getClient(const std::string& nick)
{
    for (std::vector<Client>::iterator it = _clients.begin() + 1;it != _clients.end(); it++)
    {
        if (it->getNick() == nick)
            return &(*it);
    }
    return nullptr;
}
bool Server::clientExists(const std::string& nick) {
        for (std::vector<Client>::iterator it = _clients.begin() + 1; it != _clients.end(); ++it) {
            if (it->getNick() == nick ) {
                return true; // Return true if Client with nickname found
            }
        }
        return false; // Return false if Client with nickname not found
    }

void Server::checkClientActivity() {
        const int timeout = TIMEOUT_TIME;  // Timeout in seconds (adjust as needed)
        time_t currentTime = time(nullptr);
        // Iterate through clients and check their last ping times
        std::vector<Client>::iterator cIt = _clients.begin() + 1;
        while (cIt != _clients.end())
        {
            int temp = 0;
            if (currentTime - lastPingTimes[cIt->getIndex()] > timeout && cIt->isActive()){
                std::cout << "--Client " << cIt->getIndex() << " has timed out!" << std::endl;
                temp = cIt->getIndex();
        
                for(std::map<int, time_t>::iterator it = lastPingTimes.begin(); it != lastPingTimes.end(); ++it){
                    if (it->first == temp && temp > 0){
                        std::cout<< it->first <<": Last ping: "<< (currentTime - it->second) <<"\n" ;
                        lastPingTimes.erase(it);
                        continue;
                    }
                }
                if (temp > 0){
                    handleClientDisconnect(cIt->getIndex());
                }
            }
            cIt++;
        }
}
    int Server::getClientsSize() const { 
        int count = 0;
        for (std::vector<Client>::const_iterator it = _clients.begin(); it != _clients.end(); it++)
        {
            if (it->isActive())
                count++;
        }
    return count;}

    void Server::printClients()const {
        int count = 0;
        if (DEBUG){

        for (std::vector<Client>::const_iterator clit = _clients.begin(); clit != _clients.end() ; clit++)
        {   
            std::cout << "________________________" << std::endl;
            std::cout << "Client: " << count++ << std::endl;
            std::cout << "________________________" << std::endl;
            std::cout << "Nick: " << clit->getNick() << std::endl;
            std::cout << "Username: " << clit->getUsername()<< std::endl;
            std::cout << "Socket: " << clit->getSocket()<< std::endl;
            std::cout << "Index: " << clit->getIndex()<< std::endl;
            std::cout << "Active: " << clit->isActive()<< std::endl;
            std::cout << "Authenticated: " << clit->isAuthenticated()<< std::endl;
            std::cout << "Connected: " << clit->isConnected()<< std::endl;
            std::cout << "________________________" << std::endl;

        }
        }
    }

    bool Server::indexExists(int i) const {
        for (std::vector<Client>::const_iterator clit = _clients.begin(); clit != _clients.end() ; clit++)
        {
            if(clit->getIndex() == i)
                return true;
        }
                return false;
    }