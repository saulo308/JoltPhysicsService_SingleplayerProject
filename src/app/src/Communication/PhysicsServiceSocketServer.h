#ifndef SOCKERSERVER_H
#define SOCKERSERVER_H

#include <iostream>
#include <cstring>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include "../PhysicsSimulation/PhysicsServiceImpl.h"

#define DEFAULT_BUFLEN 1048576
#define SERVER_PORT "27015"

/** 
* 
*/
class PhysicsServiceSocketServer
{
public:
    /** 
    * 
    */
    bool OpenServerSocket();

private:
    /** 
    * 
    */
    int CreateListenSocket(addrinfo* listenSocketAddrInfo);
    
    /** 
    * 
    */
    bool BindListenSocket(int listenSocketToSetup, addrinfo* listenSocketAddrInfo);
    
    /** 
    * 
    */
    int AwaitClientConnection(int listenSocket);

    /** 
    * 
    */
    ssize_t ReceiveMessageFromClient(int clientSocket, char* receivingBuffer, int receivingBufferLength);
    
    /** 
    * 
    */
    bool SendMessageToClient(int clientSocket, const char* messageBuffer);

    void SaveStepPhysicsMeasureToFile();

public:
    void InitializePhysicsSystem(const std::string initializationActorsInfo);
    std::string StepPhysicsSimulation();

private:
    PhysicsServiceImpl* PhysicsServiceImplementation = nullptr;

	std::string CurrentPhysicsStepSimulationWithoutCommsTimeMeasure = "";

    std::string decodedMessage = "";
};

#endif
