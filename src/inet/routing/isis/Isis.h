//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// This implementation is based on the IS-IS model of the ANSA project
// (https://ansa.omnetpp.org), Brno University of Technology, ported to the
// modern INET packet (Chunk) API.
//

#ifndef __INET_ISIS_H
#define __INET_ISIS_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/linklayer/ieee8022/Ieee8022LlcSocket.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/routing/base/RoutingProtocolBase.h"
#include "inet/routing/isis/IsisCommon.h"

namespace inet {
namespace isis {

//
// Implements the IS-IS routing protocol (ISO/IEC 10589 / RFC 1195). See the
// NED file for an overview. PDUs are exchanged over the link layer through an
// `Ieee8022LlcSocket`.
//
class INET_API Isis : public RoutingProtocolBase, public Ieee8022LlcSocket::ICallback
{
  protected:
    ModuleRefByPar<IInterfaceTable> ift;

    // This router's IS-IS identity, derived from the configured NET address.
    SystemId systemId;
    AreaId areaId;

    // PDUs ride directly on IEEE 802.2 LLC (SAP 0xFE), not on IP.
    Ieee8022LlcSocket llcSocket;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;

    // OperationalBase lifecycle
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    // Ieee8022LlcSocket::ICallback
    virtual void socketDataArrived(Ieee8022LlcSocket *socket, Packet *packet) override;
    virtual void socketClosed(Ieee8022LlcSocket *socket) override;

  public:
    Isis() {}

    const SystemId& getSystemId() const { return systemId; }
    const AreaId& getAreaId() const { return areaId; }
};

} // namespace isis
} // namespace inet

#endif
