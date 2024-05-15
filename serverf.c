#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/in.h>

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
    //printf("\n");
    //printf("%lld ", n);
    //printf("\n");
    //printf("%lld ", fact);
    return fact;
}

int main()
{
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    // Create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }

    memset(&serverAddr, '\0',
           sizeof(serverAddr));

    // Initialize structure

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

    // Listening
    if (listen(serverSocket, 10) == 0)
    {
        // printf("Listening...\n\n");
    }

    printf("Server listening on port 9999...\n");

    while (1)
    {
        // Accept the connection
        clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
        if (clientSocket < 0)
        {
            //perror("Accept failed");
            exit(1);
        }

        //printf("Connection accepted from %s:%d\n",
          	 //inet_ntoa(clientAddr.sin_addr),
               //ntohs(clientAddr.sin_port));

        // Forking a new process to handle the client
        int pid = fork();
        if (pid == 0)
        {
            // In the child process
            close(serverSocket); // Closing the server socket in the child process

            uint64_t num;;
            long long result;

            while (1)
            {

                // Reading the number from the client
                char buf[1024];
                long long num, result;

                ssize_t numbytes = recv(clientSocket, &buf, sizeof(buf), 0);
                if (numbytes <= 0)
                {
                    //perror("Read failed");
                    break;
                    // exit(1);
                }

                num = atoi(buf);

                result = factorial(num);

                if (send(clientSocket, &result, sizeof(result), 0) < 0)
                {
                    //perror("Send failed");
                    break;
                    // exit(1);
                }
                //printf("Got n %lld Sent res %lld\n",(long long) num, result);
            }
            // close(clientSocket);
            //  exit(0);
        }
        else if (pid < 0)
        {
            //perror("Fork failed");
        }
        else
        {
            // In the parent process, close the client socket
            // close(clientSocket);
        }
    }
    close(clientSocket);
    return 0;
}
