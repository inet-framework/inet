//
// Copyright (C) 2014 OpenSim Ltd.
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

#include "inet/common/checksum/EthernetCRC.h"
#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/physicallayer/apskradio/bitlevel/ApskPhyHeaderSerializer.h"

namespace inet {
namespace physicallayer {

Register_Serializer(ApskPhyHeader, ApskPhyHeaderSerializer);

void ApskPhyHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk, b offset, b length) const
{
    auto startPosition = stream.getLength();
    const auto& phyHeader = staticPtrCast<const ApskPhyHeader>(chunk);
    stream.writeUint16Be(b(phyHeader->getHeaderLengthField()).get());
    stream.writeUint16Be(b(phyHeader->getPayloadLengthField()).get());
    auto crcMode = phyHeader->getCrcMode();
    if (crcMode != CRC_DISABLED && crcMode != CRC_COMPUTED)
        throw cRuntimeError("Cannot serialize Apsk Phy header without turned off or properly computed CRC, try changing the value of crcMode parameter for Udp");
    stream.writeUint16Be(phyHeader->getCrc());
    //TODO write protocol

    b remainders = phyHeader->getChunkLength() - (stream.getLength() - startPosition);
    if (remainders < b(0))
        throw cRuntimeError("ApskPhyHeader length = %d smaller than required %d bytes", (int)B(phyHeader->getChunkLength()).get(), (int)B(stream.getLength() - startPosition).get());
    uint8_t remainderbits = remainders.get() % 8;
    stream.writeByteRepeatedly('?', B(remainders - b(remainderbits)).get());
    stream.writeBitRepeatedly(false, remainderbits);
}

const Ptr<Chunk> ApskPhyHeaderSerializer::deserialize(MemoryInputStream& stream, const std::type_info& typeInfo) const
{
    auto startPosition = stream.getPosition();
    auto phyHeader = makeShared<ApskPhyHeader>();
    b headerLength = b(stream.readUint16Be());
    phyHeader->setHeaderLengthField(headerLength);
    phyHeader->setChunkLength(headerLength);
    phyHeader->setPayloadLengthField(b(stream.readUint16Be()));
    auto crc = stream.readUint16Be();
    phyHeader->setCrc(crc);
    phyHeader->setCrcMode(crc == 0 ? CRC_DISABLED : CRC_COMPUTED);
    //TODO read protocol

    b curLength = stream.getPosition() - startPosition;
    b remainders = headerLength - curLength;
    if (remainders < b(0)) {
        phyHeader->markIncorrect();
    }
    else {
        uint8_t remainderbits = remainders.get() % 8;
        stream.readByteRepeatedly('?', B(remainders - b(remainderbits)).get());
        stream.readBitRepeatedly(false, remainderbits);
    }
    phyHeader->setChunkLength(stream.getPosition() - startPosition);
    return phyHeader;
}

} // namespace physicallayer
} // namespace inet

