//
// Copyright (C) 2024 Daniel Zeitler
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_MRPRELAY_H
#define __INET_MRPRELAY_H

#include "inet/linklayer/ieee8021d/relay/Ieee8021dRelay.h"
#include "inet/linklayer/mrp/MrpMacForwardingTable.h"
#include "inet/common/stlutils.h"
#include <algorithm>
#include <vector>

namespace inet {

//
// This module forward frames (~EtherFrame) based on their destination MAC addresses to appropriate interfaces.
// See the NED definition for details.
//
class INET_API MrpRelay: public Ieee8021dRelay
{
protected:
    ModuleRefByPar<MrpMacForwardingTable> mrpMacForwardingTable;

    // statistics: see finish() for details.
    int numReceivedNetworkFrames = 0;
    int numReceivedPDUsFromMRP = 0;
    int numDeliveredPDUsToMRP = 0;
    int numDispatchedNonMRPFrames = 0;
    int numDispatchedMRPFrames = 0;
    simtime_t switchingDelay;

protected:
    virtual void initialize(int stage) override;
    virtual void finish() override;

    virtual void handleUpperPacket(Packet *packet) override;
    virtual void handleLowerPacket(Packet *packet) override;
    virtual int getCcmLevel(Packet *packet);
    virtual void updatePeerAddress(NetworkInterface *incomingInterface, MacAddress sourceAddress, unsigned int vlanId) override;
    virtual bool isForwardingInterface(NetworkInterface *networkInterface) const override;
    virtual bool isMrpMulticast(MacAddress DestinationAddress);
    virtual void sendPacket(Packet *packet, const MacAddress &destinationAddress, NetworkInterface *outgoingInterface) override;

    //@{ For lifecycle
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;
    //@}

public:
    virtual MacAddress getBridgeAddress();
};

} // namespace inet

#endif

