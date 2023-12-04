#include <iostream>
#include <cstring>
#include <openssl/sha.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
const int PORT = 9001;
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
int main() {
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        std::cerr << "Error creating node socket\n";
        return 1;
    }
    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr) <= 0) {
        std::cerr << "Error setting node address\n";
        close(clientSocket);
        return 1;
    }
    if (connect(clientSocket, reinterpret_cast<struct sockaddr*>(&serverAddress), sizeof(serverAddress)) == -1) {
        std::cerr << "Error connecting to node\n";
        close(clientSocket);
        return 1;
    }
    std::cout << "Connected to node.\n";
    while (true) {
        std::string data;
        std::cout << "Type 'exit' to quit, otherwise input data: ";
        std::getline(std::cin, data);
        if (data == "exit") {
            break;
        }
        std::string hashedData = computeSHA1(data);
        send(clientSocket, hashedData.c_str(), hashedData.length(), 0);
    }
    close(clientSocket);
    return 0;
}
