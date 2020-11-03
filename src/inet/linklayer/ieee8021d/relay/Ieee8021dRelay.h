//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#ifndef __INET_IEEE8021DRELAY_H
#define __INET_IEEE8021DRELAY_H

#include "inet/linklayer/base/MacRelayUnitBase.h"

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
    bool isStpAware = false;

    typedef std::pair<MacAddress, MacAddress> MacAddressPair;

    struct Comp
    {
        bool operator() (const MacAddressPair& first, const MacAddressPair& second) const
        {
            return (first.first < second.first && first.second < second.first);
        }
    };

    bool in_range(const std::set<MacAddressPair, Comp>& ranges, MacAddress value)
    {
        return ranges.find(MacAddressPair(value, value)) != ranges.end();
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

    virtual void updatePeerAddress(NetworkInterface *incomingInterface, MacAddress sourceAddress) override;

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

