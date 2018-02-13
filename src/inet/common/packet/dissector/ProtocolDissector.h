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

#ifndef __INET_PROTOCOLDISSECTOR_H_
#define __INET_PROTOCOLDISSECTOR_H_

#include "inet/common/packet/Packet.h"
#include "inet/common/Protocol.h"

namespace inet {

class PacketDissector;

/**
 * Protocol dissector classes dissect packets into protocol specific meaningful
 * parts. The algorithm calls the visitor method exactly one time for each part
 * in order from left to right. For an aggregate packet all aggregated parts are
 * visited in the order they appear in the packet. For a fragmented packet the
 * fragment part is visited as a whole. If dissecting that part is also needed
 * then another dissector must be used for that part.
 *
 * Dissectors can handle both protocol specific and raw representations (raw
 * bytes or bits). In general, dissectors call the chunk visitor with the most
 * specific representation available for a particular protocol.
 */
class INET_API ProtocolDissector : public cObject
{
  public:
    /**
     * Dissects the packet according to the protocol implemented by this ProtocolDissector.
     */
    virtual void dissect(Packet *packet, const PacketDissector& packetDissector) const = 0;
};

class INET_API DefaultDissector : public ProtocolDissector
{
  public:
    virtual void dissect(Packet *packet, const PacketDissector& packetDissector) const override;
};

class INET_API EthernetDissector : public ProtocolDissector
{
  public:
    virtual void dissect(Packet *packet, const PacketDissector& packetDissector) const override;
};

class INET_API Ieee80211Dissector : public ProtocolDissector
{
  public:
    virtual void dissect(Packet *packet, const PacketDissector& packetDissector) const override;
};

class INET_API Ieee802LlcDissector : public ProtocolDissector
{
  public:
    virtual void dissect(Packet *packet, const PacketDissector& packetDissector) const override;
};

class INET_API ArpDissector : public ProtocolDissector
{
  public:
    virtual void dissect(Packet *packet, const PacketDissector& packetDissector) const override;
};

class INET_API Ipv4Dissector : public ProtocolDissector
{
  public:
    virtual void dissect(Packet *packet, const PacketDissector& packetDissector) const override;
};

class INET_API IcmpDissector : public ProtocolDissector
{
  public:
    virtual void dissect(Packet *packet, const PacketDissector& packetDissector) const override;
};

class INET_API UdpDissector : public ProtocolDissector
{
  public:
    virtual void dissect(Packet *packet, const PacketDissector& packetDissector) const override;
};

} // namespace

#endif // #ifndef __INET_PROTOCOLDISSECTOR_H_

