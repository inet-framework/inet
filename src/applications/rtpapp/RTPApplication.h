//
// Copyright (C) 2001 Matthias Oppitz <Matthias.Oppitz@gmx.de>
// Copyright (C) 2007 Ahmed Ayadi  <ahmed.ayadi@sophia.inria.fr>
// Copyright (C) 2010 Zoltan Bojthe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//

#ifndef __INET_RTPAPPLICATION_H
#define __INET_RTPAPPLICATION_H

#include "inet/common/INETDefs.h"
#include "inet/networklayer/contract/ipv4/IPv4Address.h"
#include "inet/common/lifecycle/ILifecycle.h"

namespace inet {

class INET_API RTPApplication : public cSimpleModule, public ILifecycle
{
  protected:
    enum SelfMsgKind {
        ENTER_SESSION,
        START_TRANSMISSION,
        STOP_TRANSMISSION,
        LEAVE_SESSION
    };

    // parameters
    const char *fileName;    // the name of the file to be transmitted
    const char *commonName;    // the CNAME of this participant.
    const char *profileName;    // the name of the used profile
    int bandwidth;    // the reserved bandwidth for rtp/rtcp in bytes/second
    int port;    // one of the udp port used
    int payloadType;    // the payload type of the data in the file
    simtime_t sessionEnterDelay;    // the delay after the application enters the session
    simtime_t transmissionStartDelay;    // the delay after the application starts the transmission
    simtime_t transmissionStopDelay;    // the delay after the application stops the transmission
    simtime_t sessionLeaveDelay;    // the delay after the application leaves the session

    // state
    IPv4Address destinationAddress;    // the address of the unicast peer or of the multicast group
    uint32 ssrc;
    bool isActiveSession;

  protected:
    virtual void initialize(int stage);
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg);
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback);

  public:
    RTPApplication();
};

} // namespace inet

#endif // ifndef __INET_RTPAPPLICATION_H

