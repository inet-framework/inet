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

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/common/checksum/EthernetCRC.h"
#include "inet/physicallayer/apskradio/bitlevel/ApskPhyHeaderSerializer.h"

namespace inet {

namespace physicallayer {

Register_Serializer(ApskPhyHeader, ApskPhyHeaderSerializer);

void ApskPhyHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk, b offset, b length) const
{
    const auto& phyHeader = staticPtrCast<const ApskPhyHeader>(chunk);
    stream.writeUint16Be(0);
    stream.writeUint16Be(phyHeader->getLengthField());
    auto crcMode = phyHeader->getCrcMode();
    if (crcMode != CRC_DISABLED && crcMode != CRC_COMPUTED)
        throw cRuntimeError("Cannot serialize Apsk Phy header without turned off or properly computed CRC, try changing the value of crcMode parameter for Udp");
    stream.writeUint16Be(phyHeader->getCrc());
    //TODO write protocol
}

const Ptr<Chunk> ApskPhyHeaderSerializer::deserialize(MemoryInputStream& stream, const std::type_info& typeInfo) const
{
    auto phyHeader = makeShared<ApskPhyHeader>();
    stream.readUint16Be();
    phyHeader->setLengthField(stream.readUint16Be());
    auto crc = stream.readUint16Be();
    phyHeader->setCrc(crc);
    phyHeader->setCrcMode(crc == 0 ? CRC_DISABLED : CRC_COMPUTED);
    return phyHeader;
    //TODO read protocol
}

} // namespace physicallayer

} // namespace inet

