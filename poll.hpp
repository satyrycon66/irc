// #include <vector>
// #include <poll.h>

// // Définir la taille initiale du vecteur de descripteurs de fichiers à surveiller
// const int MAX_FDS = 1024; // Par exemple, vous pouvez ajuster cette valeur en fonction de vos besoins

// // Dans votre fonction start() ou là où vous avez besoin du vecteur :
// std::vector<pollfd> fds(MAX_FDS); // Créez un vecteur de pollfd avec la taille spécifiée

// // Ensuite, vous pouvez initialiser chaque entrée du vecteur avec les descripteurs de fichiers appropriés.
// // Supposons que vous ayez déjà un descripteur de fichier pour le socket du serveur :
// fds[0].fd = serverSocket; // Assume que serverSocket est le descripteur de fichier du socket du serveur
// fds[0].events = POLLIN; // Surveillez les événements d'entrée sur le socket du serveur

// // Vous pouvez ajouter d'autres descripteurs de fichiers si nécessaire, par exemple des connexions client :
// for (size_t i = 1; i < MAX_FDS; ++i) {
//     fds[i].fd = /* descripteur de fichier du client */;
//     fds[i].events = POLLIN; // Par exemple, surveillez les événements d'entrée
// }