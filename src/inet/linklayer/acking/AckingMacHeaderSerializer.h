//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_ACKINGMACHEADERSERIALIZER_H
#define __INET_ACKINGMACHEADERSERIALIZER_H

#include "inet/common/packet/recorder/PcapRecorder.h"
#include "inet/common/packet/serializer/FieldsChunkSerializer.h"
#include "inet/linklayer/acking/AckingMacHeader_m.h"

namespace inet {

/**
 * Converts between AckingMacHeader and binary network byte order mac header.
 */
class INET_API AckingMacHeaderSerializer : public FieldsChunkSerializer
{
  protected:
    virtual void serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    virtual const Ptr<Chunk> deserialize(MemoryInputStream& stream) const override;

  public:
    AckingMacHeaderSerializer() : FieldsChunkSerializer() {}
};

class INET_API AckingMacToEthernetPcapRecorderHelper : public cObject, public PcapRecorder::IHelper
{
    virtual PcapLinkType protocolToLinkType(const Protocol *protocol) const;
    virtual bool matchesLinkType(PcapLinkType pcapLinkType, const Protocol *protocol) const;
    virtual Packet *tryConvertToLinkType(const Packet* packet, PcapLinkType pcapLinkType, const Protocol *protocol) const;
};

} // namespace inet

#endif // ifndef __INET_ACKINGMACHEADERSERIALIZER_H

