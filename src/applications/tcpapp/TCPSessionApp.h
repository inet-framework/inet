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

#ifndef __INET_TCPSESSIONAPP_H
#define __INET_TCPSESSIONAPP_H

#include <vector>

#include "INETDefs.h"

#include "TCPSocket.h"


/**
 * Single-connection TCP application.
 */
class INET_API TCPSessionApp : public cSimpleModule
{
  protected:
    struct Command
    {
        simtime_t tSend;
        int numBytes;
    };
    typedef std::vector<Command> CommandVector;
    CommandVector commands;

    TCPSocket socket;

    // statistics
    int packetsRcvd;
    long bytesRcvd;
    long bytesSent;
    int indicationsRcvd;
    static simsignal_t rcvdPkSignal;
    static simsignal_t sentPkSignal;
    static simsignal_t recvIndicationsSignal;

  public:
    TCPSessionApp() : cSimpleModule(65536) {}

  protected:
    virtual void parseScript(const char *script);
    virtual void waitUntil(simtime_t t);
    virtual void count(cMessage *msg);

    virtual cPacket* genDataMsg(long sendBytes);

    virtual void activity();
    virtual void finish();
};

#endif

