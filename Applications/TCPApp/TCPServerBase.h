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

#ifndef __TCPSERVERBASE_H_
#define __TCPSERVERBASE_H_

#include <map>
#include <omnetpp.h>
#include "TCPSocket.h"


class TCPServerAppBase
{
    virtual void dataArrived(int connId, void *yourPtr, cMessage *msg, bool urgent);
    virtual void established(int connId, void *yourPtr);
    virtual void peerClosed(int connId, void *yourPtr);
    virtual void closed(int connId, void *yourPtr);
    virtual void failure(int connId, void *yourPtr, int code);
    virtual void statusArrived(int connId, void *yourPtr, TCPStatusInfo *status) {delete status;}
};


/**
 * Hosts a server application.
 */
class TCPServerBase : public cSimpleModule, public TCPSocket::CallBackInterface
{
  protected:
    int port;

    typedef std::map<TCPSocket*> SocketMap;
    SocketMap socketMap;

  public:
    virtual void socketDataArrived(int connId, void *yourPtr, cMessage *msg, bool urgent);
    virtual void socketEstablished(int connId, void *yourPtr);
    virtual void socketPeerClosed(int connId, void *yourPtr);
    virtual void socketClosed(int connId, void *yourPtr);
    virtual void socketFailure(int connId, void *yourPtr, int code);
    virtual void socketStatusArrived(int connId, void *yourPtr, TCPStatusInfo *status) {delete status;}

  public:
    Module_Class_Members(TCPServerBase, cSimpleModule, 0);
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();
};

#endif


