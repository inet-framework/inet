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
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#ifndef __INET_IPV4DYNAMICNAPT_H
#define __INET_IPV4DYNAMICNAPT_H

#include "inet/common/packet/PacketFilter.h"
#include "inet/networklayer/contract/INetfilter.h"
#include "inet/networklayer/ipv4/Ipv4NatEntry.h"

namespace inet {

class INET_API Ipv4DynamicNapt : public cSimpleModule, public NetfilterBase::HookBase
{
  protected:
    INetfilter *networkProtocol = nullptr;

    PacketFilter *outgoingFilter = nullptr;
    PacketFilter *incomingFilter = nullptr;

    // (protocol_id [UDP or TCP], external_router_port) -> (private_host_address, private_host_port)
    std::map<std::pair<int, uint16_t>, std::pair<Ipv4Address, uint16_t>> portMapping;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

  public:
    virtual ~Ipv4DynamicNapt();

    virtual Result datagramPreRoutingHook(Packet *datagram) override;
    virtual Result datagramPostRoutingHook(Packet *datagram) override;

    virtual Result datagramForwardHook(Packet *datagram) override { return ACCEPT; };
    virtual Result datagramLocalInHook(Packet *datagram) override { return ACCEPT; };
    virtual Result datagramLocalOutHook(Packet *datagram) override { return ACCEPT; };
};

} // namespace inet

#endif // ifndef __INET_IPV4DYNAMICNAPT_H

