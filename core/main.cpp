#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <cstring>
#include <openssl/sha.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
const int PORT = 9001;
std::vector<int> clients;
std::mutex clientsMutex;
std::string computeSHA1(const std::string& data) {
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char*>(data.c_str()), data.length(), hash);
    char hashString[SHA_DIGEST_LENGTH * 2 + 1];
    for (int i = 0; i < SHA_DIGEST_LENGTH; ++i) {
        sprintf(hashString + (i * 2), "%02x", hash[i]);
    }
    hashString[SHA_DIGEST_LENGTH * 2] = '\0';
    return std::string(hashString);
}
void handleClient(int clientSocket) {
    std::cout << "Node connected: " << clientSocket << std::endl;
    std::lock_guard<std::mutex> lock(clientsMutex);
    clients.push_back(clientSocket);
    char buffer[1024];
    while (true) {
        ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead <= 0) {
            std::lock_guard<std::mutex> lock(clientsMutex);
            auto it = std::find(clients.begin(), clients.end(), clientSocket);
            if (it != clients.end()) {
                clients.erase(it);
            }
            close(clientSocket);
            std::cout << "Node disconnected: " << clientSocket << std::endl;
            break;
        }
        std::string data(buffer, bytesRead);
        std::string hashedData = computeSHA1(data);
        std::lock_guard<std::mutex> lock(clientsMutex);
        for (int otherClient : clients) {
            if (otherClient != clientSocket) {
                send(otherClient, hashedData.c_str(), hashedData.length(), 0);
            }
        }
    }
}
int main() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Error creating node socket\n";
        return 1;
    }
    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(PORT);
    if (bind(serverSocket, reinterpret_cast<struct sockaddr*>(&serverAddress), sizeof(serverAddress)) == -1) {
        std::cerr << "Error binding node socket\n";
        close(serverSocket);
        return 1;
    }
    if (listen(serverSocket, 10) == -1) {
        std::cerr << "Error listening on node socket\n";
        close(serverSocket);
        return 1;
    }
    std::cout << "Server listening on port " << PORT << std::endl;
    while (true) {
        int clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket == -1) {
            std::cerr << "Error accepting node connection\n";
            continue;
        }
        std::thread(handleClient, clientSocket).detach();
    }
    close(serverSocket);
    return 0;
}
