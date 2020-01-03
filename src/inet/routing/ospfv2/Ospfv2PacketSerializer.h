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

#ifndef __INET_OSPFV2PACKETSERIALIZER_H
#define __INET_OSPFV2PACKETSERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"
#include "inet/routing/ospfv2/Ospfv2Packet_m.h"

namespace inet {

namespace ospf {
class OspfPacketSerializer;
} // namespace ospf

namespace ospfv2 {

/**
 * Converts between Ospfv2Packet and binary (network byte order) OSPF data.
 */
class INET_API Ospfv2PacketSerializer : public FieldsChunkSerializer
{
  private:
    static void serializeOspfHeader(MemoryOutputStream& stream, const Ptr<const Ospfv2Packet>& ospfPacket);
    static uint16_t deserializeOspfHeader(MemoryInputStream& stream, Ptr<Ospfv2Packet>& ospfPacket);

    static void serializeRouterLsa(MemoryOutputStream& stream, const Ospfv2RouterLsa& routerLsa);
    static void deserializeRouterLsa(MemoryInputStream& stream, const Ptr<Ospfv2LinkStateUpdatePacket> updatePacket, Ospfv2RouterLsa& routerLsa);

    static void serializeNetworkLsa(MemoryOutputStream& stream, const Ospfv2NetworkLsa& networkLsa);
    static void deserializeNetworkLsa(MemoryInputStream& stream, const Ptr<Ospfv2LinkStateUpdatePacket> updatePacket, Ospfv2NetworkLsa& networkLsa);

    static void serializeSummaryLsa(MemoryOutputStream& stream, const Ospfv2SummaryLsa& summaryLsa);
    static void deserializeSummaryLsa(MemoryInputStream& stream, const Ptr<Ospfv2LinkStateUpdatePacket> updatePacket, Ospfv2SummaryLsa& summaryLsa);

    static void serializeAsExternalLsa(MemoryOutputStream& stream, const Ospfv2AsExternalLsa& asExternalLsa);
    static void deserializeAsExternalLsa(MemoryInputStream& stream, const Ptr<Ospfv2LinkStateUpdatePacket> updatePacket, Ospfv2AsExternalLsa& asExternalLsa);

    static void serializeOspfOptions(MemoryOutputStream& stream, const Ospfv2Options& options);
    static void deserializeOspfOptions(MemoryInputStream& stream, Ospfv2Options& options);

    static void copyHeaderFields(const Ptr<Ospfv2Packet> from, Ptr<Ospfv2Packet> to);

  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    Ospfv2PacketSerializer() : FieldsChunkSerializer() {}

    static void serializeLsa(MemoryOutputStream& stream, const Ospfv2Lsa& routerLsa);
    static void deserializeLsa(MemoryInputStream& stream, const Ptr<Ospfv2LinkStateUpdatePacket> updatePacket, int i);
    static void serializeLsaHeader(MemoryOutputStream& stream, const Ospfv2LsaHeader& lsaHeader);
    static void deserializeLsaHeader(MemoryInputStream& stream, Ospfv2LsaHeader& lsaHeader);

    //TODO kludge, should register Ospfv2PacketSerializer to OspfPacketSerializer later.
    friend class inet::ospf::OspfPacketSerializer;
};

} // namespace ospfv2
} // namespace inet

#endif // ifndef __INET_OSPFV2PACKETSERIALIZER_H

