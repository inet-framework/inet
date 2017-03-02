//
// Copyright (C) 2005 Christian Dankbar, Irene Ruengeler, Michael Tuexen, Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/applications/pingapp/PingPayload_m.h"
#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/common/serializer/application/PingPayloadSerializer.h"

namespace inet {

namespace serializer {

Register_Serializer(PingPayload, PingPayloadSerializer);

void PingPayloadSerializer::serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk) const
{
//    const auto& pp = std::static_pointer_cast<const PingPayload>(chunk);
//    stream.writeUint16(pp->getOriginatorId());
//    stream.writeUint16(pp->getSeqNo());
//    unsigned int datalen = pp->getDataArraySize();
//    for (unsigned int i = 0; i < datalen; i++)
//        stream.writeByte(pp->getData(i));
//    datalen = (pp->getByteLength() - 4) - datalen;
//    stream.fillNBytes(datalen, 'a');
}

std::shared_ptr<Chunk> PingPayloadSerializer::deserialize(ByteInputStream& stream) const
{
    PingPayload *pp = new PingPayload();
    pp->setOriginatorId(stream.readUint16());
    uint16_t seqno = stream.readUint16();
    pp->setSeqNo(seqno);
    pp->setByteLength(4 + stream.getRemainingSize());
    pp->setDataArraySize(stream.getRemainingSize());
    for (unsigned int i = 0; stream.getRemainingSize() > 0; i++)
        pp->setData(i, stream.readByte());
    return nullptr;
}

} // namespace serializer

} // namespace inet

