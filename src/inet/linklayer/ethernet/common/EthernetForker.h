//
// Copyright (C) 2011 OpenSim Ltd.
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

#ifndef __INET_ETHERNETFRAMECLASSIFIER_H
#define __INET_ETHERNETFRAMECLASSIFIER_H

#include "inet/common/stlutils.h"
#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleRefByPar.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/queueing/base/PacketClassifierBase.h"

namespace inet {

/**
 * Ethernet Frame classifier.
 *
 * Ethernet frames are classified as:
 * - PAUSE frames
 * - others
 */
class INET_API EthernetForker : public queueing::PacketClassifierBase, public TransparentProtocolRegistrationListener
{
  protected:
    ModuleRefByPar<IInterfaceTable> interfaceTable;
    MacAddress bridgeAddress;

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

  public:
    virtual void initialize(int stage) override;

    /**
     * Sends the incoming packet to either the first or the second gate.
     */
    virtual int classifyPacket(Packet *packet) override;

    virtual std::vector<cGate *> getRegistrationForwardingGates(cGate *gate) override;
};

} // namespace inet

#endif

