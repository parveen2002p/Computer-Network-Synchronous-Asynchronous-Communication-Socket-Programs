#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/select.h>
#include <fcntl.h>
#include <arpa/inet.h>

#define MAX_CLIENTS 1024
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
    fd_set readfds, master;
    int maxSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrLen;

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        clientSockets[i] = 0;
    }

    // Create my socket
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
    ;

    int ret;
    ret = bind(serverSocket,
               (struct sockaddr *)&serverAddr,
               sizeof(serverAddr));

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

    FD_ZERO(&master);
    FD_ZERO(&readfds);
    FD_SET(serverSocket, &master);
    maxSocket = serverSocket;

    while (1)
    {
        readfds = master;
        if (select(maxSocket + 1, &readfds, NULL, NULL, NULL) < 0)
        {
            perror("Select failed");
            exit(1);
        }

        // Checking if there is an incoming connection
        if (FD_ISSET(serverSocket, &readfds))
        {
            while (1)
            {
                clientAddrLen = sizeof(struct sockaddr_in);
                int newSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);

                if (newSocket < 0)
                {
                    break;
                    // perror("Accept failed");
                    // exit(1);
                }
                // Set the newly socket to non-blocking mode
                if (fcntl(newSocket, F_SETFL, O_NONBLOCK) < 0)
                {
                    perror("fcntl");
                    // exit(1);
                }

                // Add the new socket to the array of sockets
                for (int i = 0; i < MAX_CLIENTS; i++)
                {
                    if (clientSockets[i] == 0)
                    {
                        clientSockets[i] = newSocket;
                        if (newSocket > maxSocket)
                        {
                            maxSocket = newSocket;
                        }
                        FD_SET(newSocket, &master);
                        //printf("New connection, socket fd is %d\n", newSocket);
                        break;
                    }
                }
            }
        }
        else
        {

            // Handle all client connections
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (clientSockets[i] == 0)
                {
                    continue;
                }
                if (FD_ISSET(clientSockets[i], &readfds))
                {
                    char buf[1024];
                    uint64_t num;
                    long long result;
                    ssize_t numbytes = recv(clientSockets[i], &buf, sizeof(buf), 0);
                    num = atoi(buf);
                    if (numbytes <= 0)
                    {
                        // Connection closed or error
                        //printf("Client disconnected, socket fd is %d\n", clientSockets[i]);
                        // close(clientSockets[i]);
                        FD_CLR(clientSockets[i], &master);
                        clientSockets[i] = 0;
                    }
                    else
                    {
                        result = factorial(num);
                        send(clientSockets[i], &result, sizeof(result), 0);
                        //printf("Got n %lld Sent res %lld\n", (long long)num, result);
                    }
                }
            }
        }
    }

    // close(serverSocket);
    return 0;
}
