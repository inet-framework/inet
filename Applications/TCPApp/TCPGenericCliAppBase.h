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

#ifndef __TCPGENERICCLIAPPBASE_H_
#define __TCPGENERICCLIAPPBASE_H_

#include <vector>
#include <omnetpp.h>
#include "TCPSocket.h"
#include "GenericAppMsg_m.h"


/**
 * Base class for clients app for TCP-based request-reply protocols or apps.
 * Handles a single session (and TCP connection) at a time.
 */
class TCPGenericCliAppBase : public cSimpleModule, public TCPSocket::CallbackInterface
{
  protected:
    TCPSocket socket;

    // statistics
    int packetsRcvd;
    int bytesRcvd;

  public:
    Module_Class_Members(TCPGenericCliAppBase, cSimpleModule, 0);
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();

    virtual void handleTimer(cMessage *msg);

    /** When to connect */
    virtual simtime_t getConnectTime() = 0;

    /** ... */
    virtual simtime_t getNextSendTime() = 0;
    virtual GenericAppMsg *getMessageToSend() = 0;


    /** @name TCPSocket::CallbackInterface callback methods */
    //@{
    virtual void socketDataArrived(int connId, void *yourPtr, cMessage *msg, bool urgent);
    virtual void socketEstablished(int connId, void *yourPtr);
    virtual void socketPeerClosed(int connId, void *yourPtr);
    virtual void socketClosed(int connId, void *yourPtr);
    virtual void socketFailure(int connId, void *yourPtr, int code);
    virtual void socketStatusArrived(int connId, void *yourPtr, TCPStatusInfo *status) {delete status;}
    //@}

};

#endif


