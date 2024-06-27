#include "Server.hpp"


// Constructor
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

    if (bind(_server_fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
        throw std::runtime_error("Bind failed");
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

// Destructor
Server::~Server()
{
    std::cout << "Closing Server...\n";
    close(_server_fd);
}

// Method to start the server
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

// Method to accept a new incoming connection
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

// Method to handle data received from a client
void Server::handleClientData(int client_index)
{
    // char buffer[1024];
    bzero(buffer,sizeof(buffer));
    int valread = read(_fds[client_index].fd, buffer, sizeof(buffer) - 1);

    if (valread <= 0) {
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
        buffer[valread] = '\0';
        std::cout << "Received from client " << _fds[client_index].fd << ": " << buffer;

        // Traiter les commandes IRC
        if (strstr(buffer, "NICK ") != nullptr) {
                handleNickCommand(client_index, buffer);
        } if (strstr(buffer, "USER ") != nullptr) {
                handleUserCommand(client_index, buffer);
        } else if (strncmp(buffer, "INVITE ", 7) == 0) {
            handleInviteCommand(buffer, client_index);
        } else if (strncmp(buffer, "PASS ", 5) == 0) {
            handlePassCommand(buffer, client_index);
        }else if (strncmp(buffer, "TOPIC ", 6) == 0) {
            handleTopicCommand(buffer, client_index);
        }else if (strncmp(buffer, "CAP END ", 8) == 0) {
            std::string response = ":server CAP * END\r\n";
            send(_fds[client_index].fd, response.c_str(), response.length(), 0);
        }else if (strncmp(buffer, "CAP LS ", 7) == 0) {
            std::string response = ":server CAP * LS :multi-prefix\r\n";
        }else if (strncmp(buffer, "CAP REQ ", 8) == 0) {
            std::string response = ":server CAP * ACK :multi-prefix\r\n";
            send(_fds[client_index].fd, response.c_str(), response.length(), 0);
        }else if (strncmp(buffer, "PRIVMSG ", 8) == 0) {
            handlePrivMsgCommand(client_index, buffer,_clients[client_index]);
        }else if (strncmp(buffer, "JOIN ", 5) == 0) {
            handleJoinCommand(buffer, client_index);
        }else if (strncmp(buffer, "MODE ", 5) == 0) {
            handleModeCommand(buffer, client_index);
            // Implémenter la gestion du mode du channel
        }else if (strncmp(buffer, "PING ", 5) == 0) {
            std::string pongResponse = "PONG " + std::string(buffer + 5) + "\r\n";
            send(_fds[client_index].fd, pongResponse.c_str(), pongResponse.length(), 0);
        }else if (strncmp(buffer, "KICK ", 5) == 0) {
            handleKickCommand(buffer, client_index);        
        }else if (strncmp(buffer, "PART ", 5) == 0) {
            handlePartCommand(buffer, client_index);        
        }else {
            std::cout << "Unhandle message: " << buffer;
        }
    }
}


/*MODE*/
void Server::handleModeCommand(const char* buffer, int client_index)
{
    std::string command(buffer);  // Convert buffer to std::string for easier manipulation

    // Parse the MODE command to extract channel and mode parameters
    size_t first_space_pos = command.find(' ');
    std::string mode_params = command.substr(first_space_pos + 1); // Get the part after "MODE "

    // Extract the channel name and mode changes
    size_t second_space_pos = mode_params.find(' ');
    std::string channel = removeCRLF(mode_params.substr(0, second_space_pos)); // Extract the channel name
    std::string modes = removeCRLF(mode_params.substr(second_space_pos + 1)); // Extract the mode changes
    std::string clientNick = _clients[client_index].getNick();

   if (command.find('#') != std::string::npos)
    {
    Channel* chan = findChannel(channel);
    if (!modes.empty()&& channel != modes)
    {
        std::cout << "." << channel <<".\n";
        std::cout << "." << modes <<".\n";
    if (chan) {
        // Implement logic to parse and apply mode changes
        std::string response = ":" + clientNick + " MODE " + channel + " " + modes + "\r\n";
        send(_fds[client_index].fd, response.c_str(), response.length(), 0);
    } else {
        std::cerr << "Channel Not found: " << channel << std::endl;
        send(_clients[client_index].getSocket(), "403 ERR_NOSUCHCHANNEL", 22, 0);
    }
    }
    }
    else if (channel != modes){ ///// patch //////
    _clients[client_index].setUserMode(clientNick, modes,_fds[client_index].fd);
   
    }

}

/*KICK*/
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
    
    Client *kickTarget = chan->getOneClient(target);
    if (!kickTarget) {
        send(_clients[client_index].getSocket(), "403 ERR_NOSUCHNICK", 19, 0);
        return;
    }

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
}

void Server::sendErrorMessage(int client_index, const std::string& message) {
    // Prepare the error message to send
    std::string errorResponse = ":server ERROR : " + message + "\r\n";

    // Send the error message back to the client
    int len = errorResponse.length();
    int bytes_sent = send(_fds[client_index].fd, errorResponse.c_str(), len, 0);

    if (bytes_sent != len) {
        std::cerr << "Error sending error message to client " << _fds[client_index].fd << std::endl;
        // Handle send error if necessary
    } else {
        std::cout << "Sent error message to client " << _fds[client_index].fd << ": " << message << std::endl;
    }
}

/*PASS*/
void Server::handlePassCommand(const char* buffer, int client_index)
{
    std::cout << "Handling " << buffer;

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

        // Password accepted, proceed with appropriate actions
        // For example, set a flag indicating authentication or proceed with further operations.
        _clients[client_index].authenticate();

        // Example: If both password and username (if applicable) are verified, send welcome message
        if (_clients[client_index].getNick().empty() && !_clients[client_index].getUsername().empty()) {
            sendWelcomeMessage(client_index);
        }
    } else {
        std::cout << "Password rejected for client " << _fds[client_index].fd << std::endl;

        // Password rejected, handle accordingly (e.g., disconnect client, send error message)
        sendErrorMessage(client_index, "Incorrect password");
        handleClientDisconnect( _clients[client_index]);
    }
}

/*NICK*/
void Server::handleNickCommand(int client_index, const char* buffer)
{
    std::cout << "Handling " + std::string(buffer);
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

    _clients[client_index].setNick(nick);
    std::cout << "Handled client " << _fds[client_index].fd << "=> Nick: " << _clients[client_index].getNick() << std::endl;
    // Check if both NICK and USER commands have been received
    if (_clients[client_index].isAuthenticated() &&!_clients[client_index].getNick().empty() && !_clients[client_index].getUsername().empty()) {
        sendWelcomeMessage(client_index);
    }
}

/*USER*/
void Server::handleUserCommand(int client_index, const char* buffer)
{
    // Find the start of the username after "USER "
    std::cout << "Handling " + std::string(buffer);
    const char* username_start = strstr(buffer, "USER ") + 5;
    
    // Extract username until the next space or end of string
    std::string username;
    while (*username_start && !isspace(*username_start)) {
        username += *username_start++;
    }

    _clients[client_index].setUsername(username);

    std::cout << "Handled client " << _fds[client_index].fd << "=> User: " << _clients[client_index].getUsername() << std::endl;
    // Check if both NICK and USER commands have been received
    if (_clients[client_index].isAuthenticated() &&!_clients[client_index].getNick().empty() && !_clients[client_index].getUsername().empty()) {
        sendWelcomeMessage(client_index);
    }
}

/*DISCONNECT*/
void Server::handleClientDisconnect(Client& client)
{
    std::cout << "Client disconnected: " << client.getSocket() << std::endl;
    for (std::vector<Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (*it == client) {
            _clients.erase(it);
            close(_fds[it->getSocket()].fd);
            break;
        }
    }
}

/*Create Channel*/
void Server::createChannel(const std::string& name) {
    std::cout << "Creating Channel: " << name << std::endl;
    Channel newChannel(name);
    _channels.push_back(newChannel);

    std::cout << "Channel created: " << name << std::endl;

    // Construire et envoyer les messages JOIN et MODE à tous les clients connectés
    std::string joinMessage = ":" + std::string("server") + " JOIN " + name + "\r\n";
    std::string modeMessage = ":" + std::string("server") + " MODE " + name + " +nt\r\n";

    for (std::vector<Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        send(it->getSocket(), joinMessage.c_str(), joinMessage.length(), 0);
        // send(it->getSocket(), modeMessage.c_str(), modeMessage.length(), 0);
    }
}

/*Join Channel*/
void Server::joinChannel(const std::string& channelName, const Client& client)
{
    std::cout << "Trying to join channel: " << channelName << std::endl;
    Channel* channel = findChannel(channelName);
    if (channel) {
        std::cout << "Channel found: " << channelName << std::endl;
        try {
            channel->addClient(client);
            std::cout << "Client " << client.getSocket() << " joined channel " << channelName << std::endl;

            // Construire le message JOIN
            std::string joinMessage = ":" + client.getNick() + "!" + client.getUsername() + "@localhost JOIN " + channelName + "\r\n";

            // Envoyer le message JOIN à tous les clients du canal
            const std::vector<Client> channelClients = channel->getClients();
            for (std::vector<Client>::const_iterator it = channelClients.begin(); it != channelClients.end(); ++it) {
                std::cout << "Sending JOIN message to client: " << it->getSocket() << std::endl; // Message de débogage
                send(it->getSocket(), joinMessage.c_str(), joinMessage.length(), 0);
            }

            // Envoyer le message MODE au client qui a rejoint
            std::string modeMessage = ":server MODE " + channelName + " +nt\r\n";
            send(client.getSocket(), modeMessage.c_str(), modeMessage.length(), 0);

            // Construire et envoyer le message NAMES au client qui a rejoint
            std::string namesMessage = ":server 353 " + client.getNick() + " = " + channelName + " :";
            for (std::vector<Client>::const_iterator it = channelClients.begin(); it != channelClients.end(); ++it) {
                namesMessage += it->getNick() + " ";
            }
            namesMessage += "\r\n";
            send(client.getSocket(), namesMessage.c_str(), namesMessage.length(), 0);

            // Envoyer le message de fin NAMES au client qui a rejoint
            std::string endNamesMessage = ":server 366 " + client.getNick() + " " + channelName + " :End of /NAMES list.\r\n";
            send(client.getSocket(), endNamesMessage.c_str(), endNamesMessage.length(), 0);

        } catch (const std::exception& e) {
            std::cerr << "Failed to add client to channel " << channelName << ": " << e.what() << std::endl;
        }
    } else {
        std::cerr << "Channel not found: " << channelName << std::endl;
    }
}

/*Leave Channnel*/
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
        } else {
            std::cerr << "Client " << client.getSocket() << " is not in channel " << channelName << std::endl;
        }
    } else {
        std::cerr << "Channel not found: " << channelName << std::endl;
    }
}

/*Find Channel*/
Channel* Server::findChannel(const std::string& channelName)
{
    for (std::vector<Channel>::iterator it = _channels.begin(); it != _channels.end(); ++it) {
        if (it->getName() == removeCRLF(channelName)) {
            return &(*it);
        }
    }
    std::cerr << "Channel not found: " << channelName << std::endl;
    return NULL;
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