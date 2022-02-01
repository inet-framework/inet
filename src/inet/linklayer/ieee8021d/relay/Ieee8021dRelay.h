//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE8021DRELAY_H
#define __INET_IEEE8021DRELAY_H

#include "inet/linklayer/base/MacRelayUnitBase.h"

#include "inet/common/stlutils.h"

namespace inet {

//
// This module forward frames (~EtherFrame) based on their destination MAC addresses to appropriate interfaces.
// See the NED definition for details.
//
class INET_API Ieee8021dRelay : public MacRelayUnitBase
{
  protected:
    MacAddress bridgeAddress;
    NetworkInterface *bridgeNetworkInterface = nullptr;

    typedef std::pair<MacAddress, MacAddress> MacAddressPair;

    struct Comp {
        bool operator()(const MacAddressPair& first, const MacAddressPair& second) const
        {
            return first.first < second.first && first.second < second.first;
        }
    };

    bool in_range(const std::set<MacAddressPair, Comp>& ranges, MacAddress value)
    {
        return contains(ranges, MacAddressPair(value, value));
    }

    std::set<MacAddressPair, Comp> registeredMacAddresses;

    // statistics: see finish() for details.
    int numReceivedNetworkFrames = 0;
    int numReceivedBPDUsFromSTP = 0;
    int numDeliveredBDPUsToSTP = 0;
    int numDispatchedNonBPDUFrames = 0;
    int numDispatchedBDPUFrames = 0;

  protected:
    virtual void initialize(int stage) override;
    virtual void finish() override;

    virtual void handleUpperPacket(Packet *packet) override;
    virtual void handleLowerPacket(Packet *packet) override;

    virtual void updatePeerAddress(NetworkInterface *incomingInterface, MacAddress sourceAddress, unsigned int vlanId) override;

    virtual void sendUp(Packet *packet);
    virtual bool isForwardingInterface(NetworkInterface *networkInterface) const override;
    virtual NetworkInterface *chooseBridgeInterface();

    //@{ For lifecycle
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;
    //@}

  public:
    /**
     * Register single MAC address that this switch supports.
     */
    void registerAddress(MacAddress mac);

    /**
     * Register range of MAC addresses that this switch supports.
     */
    void registerAddresses(MacAddress startMac, MacAddress endMac);
};

} // namespace inet

#endif

