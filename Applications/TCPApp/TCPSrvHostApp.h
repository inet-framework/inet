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

#ifndef __TCPSRVHOSTAPP_H_
#define __TCPSRVHOSTAPP_H_

#include <omnetpp.h>
#include "TCPSocket.h"
#include "TCPSocketMap.h"


/**
 * Abstract base class for server processes to be used with TCPSrvHostApp.
 */
class TCPServerProcess : public cSimpleModule, public TCPSocket::CallbackInterface
{
  protected:
    TCPSocket *socket; // ptr into socketMap managed by TCPSrvHostApp
  public:
    Module_Class_Members(TCPServerProcess,cSimpleModule,0);

    /** Called by TCPSrvHostApp after creating this module */
    void setSocket(TCPSocket *sock) {socket=sock;}

    /** To be called when the server process has finished */
    void removeSocket();

    /** @name TCPSocket::CallbackInterface callback methods, to be redefined */
    //@{
    virtual void socketDataArrived(int connId, void *yourPtr, cMessage *msg, bool urgent) =0;
    virtual void socketEstablished(int connId, void *yourPtr) = 0;
    virtual void socketPeerClosed(int connId, void *yourPtr) = 0;
    virtual void socketClosed(int connId, void *yourPtr) {removeSocket();}
    virtual void socketFailure(int connId, void *yourPtr, int code) {removeSocket();}
    virtual void socketStatusArrived(int connId, void *yourPtr, TCPStatusInfo *status) {delete status;}
    //@}
};


/**
 * Hosts a server application, to be subclassed from TCPServerProcess (which
 * is a sSimpleModule). Creates one instance (using dynamic module creation)
 * for each incoming connection. More info in the corresponding NED file.
 */
class TCPSrvHostApp : public cSimpleModule
{
  protected:
    TCPSocket serverSocket;
    TCPSocketMap socketMap;
    cModuleType *srvProcType;

  public:
    Module_Class_Members(TCPSrvHostApp, cSimpleModule, 0);
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();
};

#endif


