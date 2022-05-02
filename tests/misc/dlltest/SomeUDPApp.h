//
// Copyright (C) 2004 OpenSim Ltd.
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//


#ifndef __SOMEUDPAPP_H__
#define __SOMEUDPAPP_H__

#include <vector>

#include "inet/common/INETDefs.h"

#include "UDPAppBase.h"


/**
 * A copy of UdpBasicApp, just for testing.
 * NOTE that this class is NOT declared INET_API!
 */
class SomeUDPApp : public UDPAppBase
{
  protected:
    std::string nodeName;
    int localPort, destPort;
    int msgLength;
    std::vector<Address> destAddresses;

    static int counter; // counter for generating a global number for each packet

    int numSent;
    int numReceived;

    // chooses random destination address
    virtual Address chooseDestAddr();
    virtual void sendPacket();
    virtual void processPacket(cMessage *msg);

  protected:
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);
    virtual void refreshDisplay() const override;
};

#endif

