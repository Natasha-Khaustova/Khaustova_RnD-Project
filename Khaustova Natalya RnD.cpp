#include <iostream>
#include <string>
#include <cstring>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

using namespace std;

void listenUDP(const char* address, const char* nickname) {
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len = sizeof(servaddr);
    char buffer[BUFFER_SIZE];

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Error creating socket");
        exit(1);
    }

    bzero(&servaddr, len);
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(atoi(address));

    if (bind(sockfd, (struct sockaddr*)&servaddr, len) < 0) {
        perror("Error binding");
        exit(1);
    }

    cout << "Listening for UDP packets on " << address << endl;

    while (true) {
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&cliaddr, &len);
        if (n < 0) {
            perror("Error reading UDP packet");
            continue;
        }

        buffer[n] = '\0';
        cout << "Received from " << inet_ntoa(cliaddr.sin_addr) << ": " << buffer << endl;
    }

    close(sockfd);
}

void sendUDP(const char* address, const char* nickname) {
    int sockfd;
    struct sockaddr_in servaddr;
    socklen_t len = sizeof(servaddr);
    char buffer[BUFFER_SIZE];


    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Error creating socket");
        exit(1);
    }
    int broadcastEnable = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));
    bzero(&servaddr, len);
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(address));
    servaddr.sin_addr.s_addr = inet_addr("255.255.255.255");

    while (true) {
        cout << "Enter message: ";
        cin.getline(buffer, BUFFER_SIZE);

        string fullMessage = nickname;
        fullMessage += ": ";
        fullMessage += buffer;

        if (sendto(sockfd, fullMessage.c_str(), fullMessage.length(), 0, (struct sockaddr*)&servaddr, len) < 0) {
            perror("Error sending UDP packet");
            continue;
        }
    }

    close(sockfd);
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << "Usage: " << argv[0] << " <ip_address> <port>" << endl;
        exit(1);
    }

    const char* ipAddress = argv[1];
    const char* port = argv[2];

    cout << "Enter your nickname: ";
    string nickname;
    getline(cin, nickname);

    nickname.erase(nickname.find_last_not_of(" \n\r\t")+1);

    if (fork() == 0) {
        listenUDP(port, nickname.c_str());
    } else {
        sendUDP(port, nickname.c_str());
    }

    return 0;
}
