#include "IrcServer.hpp"
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>

IRCServer::IRCServer(int port, const std::string& password)
    : port(port), password(password), serverSocket(-1), running(false) {}

IRCServer::~IRCServer() {
    if (serverSocket != -1) {
        close(serverSocket);
    }
}

bool IRCServer::start() {
    if (!createServerSocket() || !bindServerSocket() || !startListening()) {
        return false;
    }

    std::cout << "Server started and listening on port " << port << ".\n";

    running = true;

    while (running) {
        // TODO: Utiliser poll() ou équivalent pour gérer les opérations d'entrée/sortie
    }

    return true;
}

bool IRCServer::createServerSocket() {
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("Failed to create server socket");
        return false;
    }
    return true;
}

bool IRCServer::bindServerSocket() {
    struct sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Failed to bind server socket");
        return false;
    }
    return true;
}

bool IRCServer::startListening() {
    if (listen(serverSocket, SOMAXCONN) == -1) {
        perror("Failed to start listening on server socket");
        return false;
    }
    std::cout << "start listening" << std::endl;
    return true;
}

void IRCServer::acceptNewClient() {
    struct sockaddr_in clientAddr;
    socklen_t clientAddrSize = sizeof(clientAddr);
    int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);
    if (clientSocket == -1) {
        perror("Failed to accept client connection");
        return;
    }

    // Création d'une nouvelle instance de Client et ajout à la liste des clients
    clients.push_back(Client(clientSocket, "", ""));
}

void IRCServer::handleClientMessage(int clientSocket) {
    char buffer[1024];
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesReceived <= 0) {
        // Gérer la déconnexion du client
        removeClient(clientSocket);
    } else {
        // Traitement du message reçu
        std::string message(buffer, bytesReceived);
        
        // TODO: Implémenter la logique de traitement du message
    }
}

void IRCServer::broadcastMessage(const std::string& message, int senderSocket) {
    for (std::vector<Client>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
        const Client& client = *it;
        if (client.getSocket() != senderSocket) {
            sendToClient(message, client.getSocket());
        }
    }
}

void IRCServer::sendToClient(const std::string& message, int clientSocket) {
    send(clientSocket, message.c_str(), message.length(), 0);
}


void IRCServer::removeClient(int clientSocket) {
    // Find the client in the vector
    std::vector<Client>::iterator it;
    for (it = clients.begin(); it != clients.end(); ++it) {
        if (it->getSocket() == clientSocket) {
            break;
        }
    }

    // If client is found, remove it from both vector and map
    if (it != clients.end()) {
        // Remove from vector
        clients.erase(it);

        // Remove from map
        std::map<int, std::string>::iterator usernameIt = clientUsernames.find(clientSocket);
        if (usernameIt != clientUsernames.end()) {
            clientUsernames.erase(usernameIt);
        }

        // Optionally, close the socket or perform other cleanup actions
        close(clientSocket);
        
        std::cout << "Client with socket " << clientSocket << " removed." << std::endl;
    } else {
        std::cerr << "Client with socket " << clientSocket << " not found." << std::endl;
    }
}

// Dans ce fichier IRCServer.cpp :

// Nous implémentons les méthodes déclarées dans IRCServer.hpp pour créer le socket du serveur, le lier à un port, démarrer l'écoute des connexions entrantes, accepter de nouveaux clients, gérer les messages des clients, etc.
// La méthode start() est la principale boucle de gestion des connexions clients. Elle doit être complétée pour utiliser poll() ou un équivalent pour gérer les opérations d'entrée/sortie de manière non bloquante.
// Les autres méthodes sont des utilitaires pour la gestion des clients et des messages.
// Les appels système sont vérifiés pour détecter les erreurs et des messages d'erreur sont affichés en conséquence.
// Vous devrez compléter les fonctions restantes en fonction des fonctionnalités spécifiques que vous souhaitez implémenter pour votre serveur IRC.