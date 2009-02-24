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

#ifndef __INET_TCPSINKAPP_H
#define __INET_TCPSINKAPP_H

#include <omnetpp.h>
#include "INETDefs.h"



/**
 * Accepts any number of incoming connections, and discards whatever arrives
 * on them.
 */
class INET_API TCPSinkApp : public cSimpleModule
{
  protected:
    long bytesRcvd;

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();
};

#endif


