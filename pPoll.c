#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/time.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <fcntl.h> // Include for O_NONBLOCK

#define MAX_CLIENTS 4000
#define PORT 9999

long long factorial(int n)
{
    long long fact = 1;
    if (n > 20)
    {
        n = 20;
    }
    for (int i = 1; i <= n; i++)
    {
        fact *= i;
    }
    return fact;
}

int main()
{
    int serverSocket, clientSockets[MAX_CLIENTS];
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    struct pollfd pollfds[MAX_CLIENTS];

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        clientSockets[i] = -1;
        pollfds[i].fd = -1;
        pollfds[i].events = 0;
        pollfds[i].revents = 0;
    }

    // Create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }

    // Set the my server socket to non-blocking mode
    if (fcntl(serverSocket, F_SETFL, O_NONBLOCK) < 0)
    {
        perror("fcntl");
        exit(1);
    }

    // Initialize my server address structure
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("10.0.2.4");

    int ret = bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if (ret < 0)
    {
        perror("Bind failed");
        exit(1);
    }

    // Listen
    if (listen(serverSocket, 5) < 0)
    {
        perror("Listen failed");
        exit(1);
    }

    printf("Server listening on port %d...\n", PORT);

    pollfds[0].fd = serverSocket;
    pollfds[0].events = POLLIN;

    while (1)
    {
        int ret = poll(pollfds, MAX_CLIENTS, -1);
        if (ret < 0)
        {
            perror("Poll failed");
            // exit(1);
        }

        if (pollfds[0].revents & POLLIN)
        {
            while (1)
            {
                int newSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
                if (newSocket < 0)
                {
                    // perror("Accept failed");
                    break;
                    // exit(1);
                }

                // Set the new socket to non-blocking mode
                if (fcntl(newSocket, F_SETFL, O_NONBLOCK) < 0)
                {
                    perror("fcntl");
                    // exit(1);
                }

                int i;
                for (i = 1; i < MAX_CLIENTS; i++)
                {
                    if (clientSockets[i] == -1)
                    {
                        clientSockets[i] = newSocket;
                        pollfds[i].fd = newSocket;
                        pollfds[i].events = POLLIN;
                        //printf("New connection, socket fd is %d\n", newSocket);
                        break;
                    }
                }

                if (i == MAX_CLIENTS)
                {
                    fprintf(stderr, "Too many clients\n");
                    // close(newSocket);
                }
            }
        }
        else
        {
            for (int i = 1; i < MAX_CLIENTS; i++)
            {
                if (clientSockets[i] == -1)
                {
                    continue;
                }
                if (pollfds[i].revents & POLLIN)
                {
                    char buf[1024];
                    uint64_t num;
                    long long result;
                    ssize_t numbytes = recv(clientSockets[i], &buf, sizeof(buf), 0);
                    if (numbytes <= 0)
                    {
                        // perror("ERROR");
                        // exit(1);
                        clientSockets[i] = -1;
                        pollfds[i].fd = -1;
                        pollfds[i].events = 0;
                        continue;
                    }
                    num = atoi(buf);
                    // memcpy(&num, buf, sizeof(long long));
                    result = factorial(num);
                    // memcpy(buf, &result, sizeof(long long));
                    send(clientSockets[i], &result, sizeof(result), 0);
                    //printf("Got n %lld Sent res %lld\n",(long long) num, result);
                }
            }
        }
    }

    // close(serverSocket);
    return 0;
}
