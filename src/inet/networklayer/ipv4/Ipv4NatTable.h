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

#ifndef __INET_IPV4NATTABLE_H
#define __INET_IPV4NATTABLE_H

#include "inet/common/packet/PacketFilter.h"
#include "inet/networklayer/contract/INetfilter.h"
#include "inet/networklayer/ipv4/Ipv4NatEntry_m.h"

namespace inet {

class INET_API Ipv4NatTable : public cSimpleModule, public NetfilterBase::HookBase
{
  protected:
    cXMLElement *config = nullptr;
    INetfilter *networkProtocol = nullptr;

    std::multimap<INetfilter::IHook::Type, std::pair<PacketFilter *, Ipv4NatEntry>> natEntries;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;
    virtual void parseConfig();
    virtual Result processPacket(Packet *packet, INetfilter::IHook::Type type);

  public:
    virtual ~Ipv4NatTable();
    virtual Result datagramPreRoutingHook(Packet *datagram) override { return processPacket(datagram, PREROUTING); }
    virtual Result datagramForwardHook(Packet *datagram) override { return processPacket(datagram, FORWARD); }
    virtual Result datagramPostRoutingHook(Packet *datagram) override { return processPacket(datagram, POSTROUTING); }
    virtual Result datagramLocalInHook(Packet *datagram) override { return processPacket(datagram, LOCALIN); }
    virtual Result datagramLocalOutHook(Packet *datagram) override { return processPacket(datagram, LOCALOUT); }
};

} // namespace inet

#endif // ifndef __INET_IPV4NATTABLE_H

