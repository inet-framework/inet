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


/**
 * Base class for clients app for TCP-based request-reply protocols or apps.
 * Handles a single session (and TCP connection) at a time.
 */
class TCPGenericCliAppBase : public cSimpleModule
{
  protected:
    TCPSocket socket;

    // statistics
    int packetsRcvd;
    int bytesRcvd;
    int indicationsRcvd;

  protected:
    void waitUntil(simtime_t t);
    void count(cMessage *msg);

  public:
    Module_Class_Members(TCPGenericCliAppBase, cSimpleModule, 16384);
    virtual void activity();
    virtual void finish();
};

#endif


