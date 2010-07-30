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

#ifndef __INET_TCPECHOAPP_H
#define __INET_TCPECHOAPP_H

#include <omnetpp.h>

#include "INETDefs.h"


/**
 * Accepts any number of incoming connections, and sends back whatever
 * arrives on them.
 */
class INET_API TCPEchoApp : public cSimpleModule
{
  public:
    TCPEchoApp();
    ~TCPEchoApp();

  protected:
    simtime_t delay;
    double echoFactor;
    bool useExplicitRead;
    bool sendNotificationsEnabled;
    ulong readBufferSize;

    bool waitingData;
    long bytesInSendQueue;
    long sendBufferLimit;

    // statistics:
    long bytesRcvd;
    long bytesSent;
    long bytesSentAndAcked;
    cOutVector *bytesRcvdVector;
    cOutVector *bytesSentVector;
    cOutVector *bytesSentAndAckedVector;

  protected:
    virtual void sendDown(cMessage *msg);
    virtual void sendDownReadCmd(cMessage *msg, int connId);

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();
};

#endif


