#include "IrcServer.hpp"
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]) {
    // Vérifiez le nombre d'arguments de la ligne de commande
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
        return 1;
    }

    // Récupérez le numéro de port et le mot de passe à partir des arguments de la ligne de commande
    int port = std::atoi(argv[1]);
    std::string password = argv[2];

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Erreur lors de la création du socket" << std::endl;
        return 1;
    }

    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(port);

    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        std::cerr << "Erreur lors du bind" << std::endl;
        return 1;
    }

    listen(serverSocket, 5);

    std::cout << "Serveur IRC démarré sur le port " << port << " avec le mot de passe : " << password << std::endl;

    int clientSocket = accept(serverSocket, NULL, NULL);
    if (clientSocket < 0) {
        std::cerr << "Erreur lors de l'acceptation du client" << std::endl;
        return 1;
    }

    char buffer[1024];

    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) {
            
            std::cerr << "Erreur de réception" << std::endl;
            break;
        }
        
        std::cout << "Données reçues du client : " << buffer << std::endl;

        std::string message = "Réponse du serveur : Merci pour votre message\n";
        send(clientSocket, message.c_str(), message.length(), 0);
    }

    close(serverSocket);
    close(clientSocket);

    return 0;
}



// int main(int argc, char *argv[]) {
//     if (argc != 3) {
//         std::cerr << "Usage: " << argv[0] << " <port> <password>\n";
//         return 1;
//     }

//     int port = std::stoi(argv[1]);
//     std::string password = argv[2];

//     IRCServer ircServer(port, password);

//     if (!ircServer.start()) {
//         std::cerr << "Failed to start IRC server.\n";
//         return 1;
//     }

//     return 0;
// }

// Dans ce fichier main.cpp :

// Nous vérifions d'abord que deux arguments ont été fournis sur la ligne de commande : le port et le mot de passe.
// Ensuite, nous convertissons le port en un entier et stockons le mot de passe dans une chaîne de caractères.
// Nous créons ensuite une instance de IRCServer en utilisant le port et le mot de passe fournis.
// Enfin, nous démarrons le serveur IRC en appelant la méthode start() de IRCServer.
// Assurez-vous d'adapter ce code à votre implémentation spécifique de la classe IRCServer et à votre logique métier pour gérer les clients et les canaux IRC.