#ifndef IRC_SERVER_HPP
#define IRC_SERVER_HPP

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "Client.hpp"

class IRCServer {
private:
        int port;
    std::string password;
    int serverSocket;
    bool running;
    std::vector<Client> clients;
    std::map<int, std::string> clientUsernames; // Mapping des sockets clients aux noms d'utilisateur
    
    // Méthodes privées pour la gestion des sockets et des clients
    bool createServerSocket();
    bool bindServerSocket();
    bool startListening();
    void acceptNewClient();
    void handleClientMessage(int clientSocket);
    void removeClient(int clientSocket);
    void broadcastMessage(const std::string& message, int senderSocket);
    void sendToClient(const std::string& message, int clientSocket);

public:
    IRCServer(int port, const std::string& password);
    ~IRCServer();

    bool start();
};

#endif // IRC_SERVER_HPP

// Dans ce fichier IRCServer.hpp :

// Nous déclarons la classe IRCServer qui représente votre serveur IRC.
// Les membres privés comprennent des variables pour le port, le mot de passe, les sockets du serveur et des clients, et d'autres données nécessaires pour gérer les clients IRC.
// Nous déclarons également des méthodes privées pour la gestion des sockets et des clients, telles que la création et la liaison du socket du serveur, l'acceptation de nouveaux clients, la gestion des messages des clients, etc.
// La classe IRCServer expose un constructeur prenant le port et le mot de passe comme arguments, ainsi qu'une méthode start() pour démarrer le serveur IRC.
// Vous devrez implémenter les méthodes déclarées dans ce fichier IRCServer.hpp dans le fichier IRCServer.cpp correspondant, en tenant compte des exigences spécifiques de votre serveur IRC.