//
// Copyright 2004 Andras Varga
//
// This library is free software, you can redistribute it and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//

#ifndef __TCPSERVERAPP_H_
#define __TCPSERVERAPP_H_

#include <omnetpp.h>
#include "TCPSocket.h"
#include "TCPSocketMap.h"


class TCPServerProcess : public cSimpleModule, public TCPSocket::CallbackInterface
{
  private:
    TCPSocket *socket; // ptr into socketMap managed by TCPServerApp
  public:
    void removeSocket();
    virtual void socketDataArrived(int connId, void *yourPtr, cMessage *msg, bool urgent);
    virtual void socketEstablished(int connId, void *yourPtr);
    virtual void socketPeerClosed(int connId, void *yourPtr);
    virtual void socketClosed(int connId, void *yourPtr) {removeSocket();}
    virtual void socketFailure(int connId, void *yourPtr, int code) {removeSocket();}
    virtual void socketStatusArrived(int connId, void *yourPtr, TCPStatusInfo *status) {delete status;}

};


/**
 * Hosts a server application.
 */
class TCPServerApp : public cSimpleModule
{
  protected:
    TCPSocket serverSocket;
    TCPSocketMap socketMap;

  public:
    Module_Class_Members(TCPServerApp, cSimpleModule, 0);
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();
};

#endif


