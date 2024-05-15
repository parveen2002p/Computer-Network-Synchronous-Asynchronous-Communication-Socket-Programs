#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <arpa/inet.h>

long long factorial(long long n)
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
    // printf("\n");
    // printf("got %lld ", n);
    // printf(" send %lld ", fact);
    return fact;
}

// Function handle a client connection
void *handleClient(void *clientSocketPtr)
{
    int clientSocket = *((int *)clientSocketPtr);
    free(clientSocketPtr); // Free the memory
    pthread_detach(pthread_self());

    while (1)
    {

        char buf[1024];
        uint64_t num;
        long long result;

        ssize_t numbytes = recv(clientSocket, &buf, sizeof(buf), 0);
        if (numbytes <= 0)
        {
            //perror("Read failed");
            break;
        }

        num = atoi(buf);

        result = factorial(num);

        if (send(clientSocket, &result, sizeof(result), 0) < 0)
        {
            //perror("Send failed");
            break;
        }
        //printf("Got n %lld Sent res %lld\n", (long long)num, result);
    }

    // close(clientSocket);
    pthread_exit(NULL);
}

int main()
{
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    pthread_t thread;

    // Create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }

    // Initialize structure
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(9999);
    serverAddr.sin_addr.s_addr = inet_addr("10.0.2.4");

    // Bind
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
    if (listen(serverSocket, 10) < 0)
    {
        perror("Listen failed");
        exit(1);
    }

    printf("Server listening on port 9999...\n");

    while (1)
    {
        // Accepting the connection
        clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
        if (clientSocket < 0)
        {
            //perror("Accept failed");
            exit(1);
        }

        // Creating a new thread
        int *clientSocketPtr = (int *)malloc(sizeof(int));
        *clientSocketPtr = clientSocket;
        if (pthread_create(&thread, NULL, handleClient, (void *)clientSocketPtr) != 0)
        {
            //perror("Thread creation failed");
            // close(clientSocket);
        }
    }

    close(serverSocket);
    return 0;
}
