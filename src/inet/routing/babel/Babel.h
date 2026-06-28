//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// Babel routing protocol (RFC 6126) ported from the ANSAINET project.
// Original authors: Vit Rek, Vladimir Vesely (Brno University of Technology).
//

#ifndef __INET_BABEL_H
#define __INET_BABEL_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/routing/babel/BabelDefs.h"
#include "inet/routing/base/RoutingProtocolBase.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"

namespace inet {
namespace babel {

/**
 * Implementation of the Babel routing protocol (RFC 6126), ported from the
 * ANSAINET project to the modern INET packet (Chunk) API.
 *
 * This is the scaffold: it owns the UDP socket and the protocol's databases as
 * C++ member objects, follows the OperationalBase lifecycle, and binds UDP port
 * 6696. The wire format, neighbour management and route selection are added in
 * later phases.
 */
class INET_API Babel : public RoutingProtocolBase
{
  protected:
    // environment
    cModule *host = nullptr;
    ModuleRefByPar<IInterfaceTable> ift;

    // configuration
    int udpPort = defval::PORT;

    // state
    rid routerId;
    uint16_t seqno = 0;
    UdpSocket socket;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;

    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

  public:
    Babel() {}
    virtual ~Babel();
};

} // namespace babel
} // namespace inet

#endif
