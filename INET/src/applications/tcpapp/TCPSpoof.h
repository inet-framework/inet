//
// Copyright 2006 Andras Varga
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

#ifndef __INET_TCPSPOOF_H
#define __INET_TCPSPOOF_H

#include <omnetpp.h>
#include "INETDefs.h"
#include "TCPSegment.h"
#include "IPAddressResolver.h"


/**
 * Sends fabricated TCP packets.
 */
class INET_API TCPSpoof : public cSimpleModule
{
  protected:
    virtual void sendToIP(TCPSegment *tcpseg, IPvXAddress src, IPvXAddress dest);
    virtual unsigned long chooseInitialSeqNum();
    virtual void sendSpoofPacket();

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};

#endif


