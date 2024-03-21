#include "PhysicsServiceSocketServer.h"
#include <sstream>
#include <chrono>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

bool PhysicsServiceSocketServer::OpenServerSocket()
{
    // Get this server (local) addrinfo
    // This will get the server addr as localhost
    addrinfo hints, *addrInfoResult;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    int getAddrInfoReturnValue = getaddrinfo(NULL, SERVER_PORT, &hints, &addrInfoResult);
    if (getAddrInfoReturnValue != 0)
    {
        printf("getaddrinfo failed with error: %s\n", gai_strerror(getAddrInfoReturnValue));
        return false;
    }
    
    // Create a socket for the server to listen for client connections.
    int serverListenSocket = CreateListenSocket(addrInfoResult);
    if (serverListenSocket == -1) 
    {
        freeaddrinfo(addrInfoResult);
        return false;
    }

    // Setup the TCP listening socket
    if(!BindListenSocket(serverListenSocket, addrInfoResult))
    {
        freeaddrinfo(addrInfoResult);
        return false;
    }

    // Free addrinfo as we don't need it anymore
    freeaddrinfo(addrInfoResult);

    /*PhysicsServiceImplementation = new PhysicsServiceImpl();
    const std::string test = "Init\n1;0;0;0;0;0;0\n2;0;0;0;0;0;0\n";
    InitializePhysicsSystem(test);*/

    // Await for the client connection on the listening socket
    int clientSocket = AwaitClientConnection(serverListenSocket);
    if (clientSocket == -1) 
    {
        return false;
    }

    // Once the connection has been established, close the listening socket as we 
    // won't need it anymore (the communication is done via the new client socket)
    close(serverListenSocket);

    PhysicsServiceImplementation = new PhysicsServiceImpl();
    CurrentPhysicsStepSimulationWithoutCommsTimeMeasure = "";

    // Receive until the peer shuts down the connection
    ssize_t messageReceivalReturnValue = 0;
    do 
    {
        // Will stall this process thread until receives a new message from client
        // The message will be passed on the buffer and the amount of received bytes is the return value
        // @note Passing a default buffer len. For bigger messages, this should be increased
        char receivingBuffer[DEFAULT_BUFLEN];
        messageReceivalReturnValue = ReceiveMessageFromClient(clientSocket, receivingBuffer, DEFAULT_BUFLEN);
        if(messageReceivalReturnValue <= 0)
        {
            break;
        }

        // Debug: Print received message
        if(messageReceivalReturnValue > 0)
        {
            //  (DEBUG) Print received message
            decodedMessage += std::string(receivingBuffer, receivingBuffer + messageReceivalReturnValue);
            std::cout << "Decoded message:" << decodedMessage << "\n=======\n";

            if((decodedMessage.find("Init") != std::string::npos) && (decodedMessage.find("EndMessage") != std::string::npos))
            {
                InitializePhysicsSystem(decodedMessage);
                SendMessageToClient(clientSocket, "OK");
                decodedMessage = "";
                continue;
            }

            if(decodedMessage.find("Step") != std::string::npos)
            {
                // Get pre step physics time
                std::chrono::steady_clock::time_point preStepPhysicsTime = std::chrono::steady_clock::now();

                std::string stepSimulationResult = StepPhysicsSimulation();
                stepSimulationResult += "OK\n";

                // Get post physics communication time
                std::chrono::steady_clock::time_point postStepPhysicsTime = std::chrono::steady_clock::now();

                // Calculate the microsseconds all step physics simulation
                // (considering communication )took
                std::stringstream ss;
                ss << std::chrono::duration_cast<std::chrono::microseconds>(postStepPhysicsTime - preStepPhysicsTime).count();
                const std::string elapsedTime = ss.str();

                // Append the delta time to the current step measurement
                CurrentPhysicsStepSimulationWithoutCommsTimeMeasure += elapsedTime + "\n";

                SendMessageToClient(clientSocket, stepSimulationResult.c_str());
                decodedMessage = "";
                continue;
            }
            //std::cout << "Unknown message: " << decodedMessage << std::endl;
            //SendMessageToClient(clientSocket, "Unkown message error");
        }
    } while (messageReceivalReturnValue > 0);

    // Save step physics measurement to file
    SaveStepPhysicsMeasureToFile();

    // shutdown the connection since we're done
    const int shutdownResult = shutdown(clientSocket, SHUT_RDWR);
    if (shutdownResult == -1) 
    {
        printf("Shutdown failed with error: %s\n", strerror(errno));
        close(clientSocket);
        return false;
    }

    // Finished work, clean up
    close(clientSocket);

    return true;
}

int PhysicsServiceSocketServer::CreateListenSocket(addrinfo* listenSocketAddrInfo)
{
    // Create a new socket using the addr info 
    int newListenSocket = socket(listenSocketAddrInfo->ai_family, listenSocketAddrInfo->ai_socktype, listenSocketAddrInfo->ai_protocol);

    // Check if creation was successful
    if (newListenSocket == -1) 
    {
        printf("Socket failed with error: %s\n", strerror(errno));
        return -1;
    }

    return newListenSocket;
}

bool PhysicsServiceSocketServer::BindListenSocket(int listenSocketToSetup, addrinfo* listenSocketAddrInfo)
{
    // Bind the listen socket to the addrinfo
    const int bindReturnValue = 
        bind(listenSocketToSetup, listenSocketAddrInfo->ai_addr, listenSocketAddrInfo->ai_addrlen);

    // Check for errors
    if (bindReturnValue == -1) 
    {
        printf("Bind failed with error: %s\n", strerror(errno));
        close(listenSocketToSetup);
        return false;
    }

    return true;
}

int PhysicsServiceSocketServer::AwaitClientConnection(int listenSocket)
{
    const int listenReturnValue = 
        listen(listenSocket, SOMAXCONN);

    if (listenReturnValue == -1) 
    {
        printf("Listen failed with error: %s\n", strerror(errno));
        close(listenSocket);
        return -1;
    }

    // Await a client connection to the listening socket
    printf("Awaiting client connection...\n");

    // Once connection is done, the library will create a new socket for it
    int connectedClientSocket = accept(listenSocket, NULL, NULL);

    // Check for errors on the client socket creation
    if (connectedClientSocket == -1) 
    {
        printf("Socket accept failed with error: %s\n", strerror(errno));
        close(listenSocket);
        return -1;
    }

    return connectedClientSocket;
}

ssize_t PhysicsServiceSocketServer::ReceiveMessageFromClient(int clientSocket, char* receivingBuffer, int receivingBufferLength)
{
    // This call will stall this process thread until we receive a message from the client (game)
    // The message received will be on "receivingBuffer", given the buffer length
    // The returning value will be the amount of bytes on the received message
    const ssize_t bytesReceivedAmount =
        recv(clientSocket, receivingBuffer, receivingBufferLength, 0);

    // If received 0, that means the client is requesting to close the connection
    if(bytesReceivedAmount == 0)
    {
        printf("Received a close connection message (0 bytes)\n");
        printf("Closing connection...\n");
        return 0;
    }

    // If received a value > 0, we have a valid message from the client
    if(bytesReceivedAmount > 0)
    {
        //printf("Received bytes amount: %ld\n", bytesReceivedAmount);

        // return the amount of received bytes
        return bytesReceivedAmount;
    }

    // If received value is < 0, we have an error. Let's close the connection
    printf("recv failed with error: %s\n", strerror(errno));
    close(clientSocket);

    return -1;
}

bool PhysicsServiceSocketServer::SendMessageToClient(int clientSocket, const char* messageBuffer)
{
    // Send the given message to the client
    const ssize_t sendReturnValue = send(clientSocket, messageBuffer, strlen(messageBuffer), 0);

    // Check for sending error
    if (sendReturnValue == -1) 
    {
        printf("send failed with error: %s\n", strerror(errno));
        close(clientSocket);
        return false;
    }

    //printf("Bytes sent: %ld\n", sendReturnValue);
    return true;
}

void PhysicsServiceSocketServer::InitializePhysicsSystem(const std::string initializationActorsInfo)
{
    if(!PhysicsServiceImplementation)
    {
        std::cout << "No physics service implementation valid to init physics system.\n";
        return;
    }

    PhysicsServiceImplementation->InitPhysicsSystem(initializationActorsInfo);
}

std::string PhysicsServiceSocketServer::StepPhysicsSimulation()
{
    if(!PhysicsServiceImplementation)
    {
        std::cout << "No physics service implementation valid to step physics simulation.\n";
        return "";
    }

    return PhysicsServiceImplementation->StepPhysicsSimulation();
}

void PhysicsServiceSocketServer::SaveStepPhysicsMeasureToFile()
{
    std::string directoryName = "StepPhysicsMeasure";

    // Create the directory
    fs::create_directory(directoryName);

    std::string fileName = "/StepPhysicsMeasureWithoutCommsOverhead_Remote_Spheres_.txt";
    std::string fullPath = directoryName + "/" + fileName;

    // Open the file in output mode
    std::ofstream file(fullPath);

    if (file.is_open()) { // Check if the file was opened successfully
        file << CurrentPhysicsStepSimulationWithoutCommsTimeMeasure; // Write the string to the file
        file.close(); // Close the file
        std::cout << "Data written to file successfully." << std::endl;
    } else {
        std::cout << "Failed to open the file." << std::endl;
    }
}
