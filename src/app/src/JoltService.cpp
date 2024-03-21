#include "Communication/PhysicsServiceSocketServer.h"

int main(int argc, char** argv) 
{
    // Open socket acting as a server socket
    // The proxy will await for the game's connection on him
    PhysicsServiceSocketServer* PhysicsServiceServer = new PhysicsServiceSocketServer();
    if(!PhysicsServiceServer)
    {
        printf("Error when creating socket server.\n");
        return 0;
    }
    
    // Open server socket to listen for client's (game) connection
    const bool bWasSocketConnectionSuccess = PhysicsServiceServer->OpenServerSocket();

    // Check for errors
    if(!bWasSocketConnectionSuccess)
    {
        printf("Could not open socket connection. Check logs.\n");
        return 0;
    }


    return 0;
}
