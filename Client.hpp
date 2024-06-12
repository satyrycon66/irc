#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <vector>

class Client {
private:
    int socket;
    std::string username;
    std::string nickname;
    std::vector<std::string> channels;

public:
    Client(int socket, const std::string& username, const std::string& nickname);
    ~Client();

    int getSocket() const;
    std::string getUsername() const;
    std::string getNickname() const;
    std::vector<std::string> getChannels() const;

    void joinChannel(const std::string& channel);
    void leaveChannel(const std::string& channel);
    bool isInChannel(const std::string& channel) const;
};

#endif // CLIENT_HPP

// Dans ce fichier Client.hpp :

// Nous déclarons la classe Client qui représente un client IRC connecté au serveur.
// Les membres privés comprennent un socket pour la communication avec le client, ainsi que des données sur le nom d'utilisateur, le surnom et les canaux auxquels le client est connecté.
// Nous déclarons des méthodes pour accéder aux informations du client telles que le socket, le nom d'utilisateur, le surnom et les canaux, ainsi que des méthodes pour rejoindre, quitter et vérifier l'appartenance à un canal.
// Le constructeur Client prend un socket, un nom d'utilisateur et un surnom comme arguments pour initialiser un nouvel objet Client.
// Le destructeur ~Client() peut être utilisé pour nettoyer les ressources associées à un client lorsque l'objet Client est détruit.