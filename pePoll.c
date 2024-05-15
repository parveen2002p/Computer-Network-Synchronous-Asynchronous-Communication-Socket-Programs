#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <fcntl.h> // Include for O_NONBLOCK

#define MAX_CLIENTS 4000
#define PORT 9999

// Function to calculate the factorial
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
	printf("PID: %d ",getpid());
    int serverSocket;
    int clientSockets[MAX_CLIENTS];
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    // for (int i = 0; i < MAX_CLIENTS; i++) {
    //     clientSockets[i] = -1;
    // }

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

    int epollFd = epoll_create1(0);
    if (epollFd == -1)
    {
        perror("epoll_create1");
        exit(1);
    }

    struct epoll_event event, events[MAX_CLIENTS];
    event.events = EPOLLIN;
    event.data.fd = serverSocket;

    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, serverSocket, &event) == -1)
    {
        perror("epoll_ctl");
        exit(1);
    }

    while (1)
    {
        int numEvents = epoll_wait(epollFd, events, MAX_CLIENTS, -1);
        if (numEvents == -1)
        {
            perror("epoll_wait");
            // exit(1);
        }

        for (int i = 0; i < numEvents; i++)
        {
            int fd = events[i].data.fd;

            if (fd == serverSocket)
            {
                int newSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
                if (newSocket < 0)
                {
                    perror("Accept failed");
                    // exit(1);
                }

                if (newSocket >= 0)
                { // added later
                    // Set the new socket to non-blocking mode
                    if (fcntl(newSocket, F_SETFL, O_NONBLOCK) < 0)
                    {
                        perror("fcntl");
                        // exit(1);
                    }

                    // int j;
                    // for (j = 0; j < MAX_CLIENTS; j++) {
                    // if (clientSockets[j] == -1) {
                    // clientSockets[j] = newSocket;

                    event.events = EPOLLIN | EPOLLET;
                    event.data.fd = newSocket;
                    //printf("New connection, socket fd is %d\n", newSocket);
                    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, newSocket, &event) == -1)
                    {
                        perror("epoll_ctl");
                        // exit(1);
                    }

                    // break;
                    //}
                    //}

                    // if (j == MAX_CLIENTS) {
                    //     fprintf(stderr, "Too many clients\n");
                    //     //close(newSocket);
                    // }
                }
            }
            else
            {
                char buf[1024];
                uint64_t num;
                long long result;
                ssize_t numbytes = recv(fd, &buf, sizeof(buf), 0);
                if (numbytes <= 0)
                {
                    // perror("ERROR");
                    // exit(1);
                    clientSockets[i] = -1;
                    continue;
                }
                num = atoi(buf);
                result = factorial(num);
                // memcpy(buf, &result, sizeof(long long));
                send(fd, &result, sizeof(result), 0);
                //printf("Got n %lld Sent res %lld\n", (long long)num, result);
            }
        }
    }

    // close(serverSocket);
    return 0;
}
