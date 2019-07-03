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

#ifndef __INET_OSPFPACKETSERIALIZER_H
#define __INET_OSPFPACKETSERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"
#include "inet/routing/ospfv2/OspfPacket_m.h"

namespace inet {

namespace ospf {

/**
 * Converts between OspfPacket and binary (network byte order) OSPF data.
 */
class INET_API OspfPacketSerializer : public FieldsChunkSerializer
{
  private:
    static void serializeOspfHeader(MemoryOutputStream& stream, const IntrusivePtr<const OspfPacket>& ospfPacket);
    static uint16_t deserializeOspfHeader(MemoryInputStream& stream, IntrusivePtr<OspfPacket>& ospfPacket);

    static void serializeRouterLsa(MemoryOutputStream& stream, const OspfRouterLsa& routerLsa);
    static void deserializeRouterLsa(MemoryInputStream& stream, const Ptr<OspfLinkStateUpdatePacket> updatePacket, OspfRouterLsa& routerLsa);

    static void serializeNetworkLsa(MemoryOutputStream& stream, const OspfNetworkLsa& networkLsa);
    static void deserializeNetworkLsa(MemoryInputStream& stream, const Ptr<OspfLinkStateUpdatePacket> updatePacket, OspfNetworkLsa& networkLsa);

    static void serializeSummaryLsa(MemoryOutputStream& stream, const OspfSummaryLsa& summaryLsa);
    static void deserializeSummaryLsa(MemoryInputStream& stream, const Ptr<OspfLinkStateUpdatePacket> updatePacket, OspfSummaryLsa& summaryLsa);

    static void serializeAsExternalLsa(MemoryOutputStream& stream, const OspfAsExternalLsa& asExternalLsa);
    static void deserializeAsExternalLsa(MemoryInputStream& stream, const Ptr<OspfLinkStateUpdatePacket> updatePacket, OspfAsExternalLsa& asExternalLsa);

    static void serializeOspfOptions(MemoryOutputStream& stream, const OspfOptions& options);
    static void deserializeOspfOptions(MemoryInputStream& stream, OspfOptions& options);

    static void copyHeaderFields(const Ptr<OspfPacket> from, Ptr<OspfPacket> to);

  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    OspfPacketSerializer() : FieldsChunkSerializer() {}

    static void serializeLsa(MemoryOutputStream& stream, const OspfLsa& routerLsa);
    static void deserializeLsa(MemoryInputStream& stream, const Ptr<OspfLinkStateUpdatePacket> updatePacket, int i);
    static void serializeLsaHeader(MemoryOutputStream& stream, const OspfLsaHeader& lsaHeader);
    static void deserializeLsaHeader(MemoryInputStream& stream, OspfLsaHeader& lsaHeader);
};

} // namespace ospf
} // namespace inet

#endif // ifndef __INET_OSPFPACKETSERIALIZER_H

