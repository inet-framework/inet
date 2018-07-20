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
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211PhyHeader_m.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211PhyHeaderSerializer.h"

namespace inet {

namespace  physicallayer {

Register_Serializer(Ieee80211PhyHeader, Ieee80211PhyHeaderSerializer);
Register_Serializer(Ieee80211OfdmPhyHeader, Ieee80211PhyHeaderSerializer);

void Ieee80211PhyHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& phyHeader = staticPtrCast<const Ieee80211PhyHeader>(chunk);
    if (auto ofdmPhyHeader = dynamicPtrCast<const Ieee80211OfdmPhyHeader>(chunk)) {
        stream.writeUint4(ofdmPhyHeader->getRate());
        stream.writeBit(false);
        stream.writeUint4((uint8_t)(ofdmPhyHeader->getLengthField().get() >> 8));
        stream.writeUint4((uint8_t)(ofdmPhyHeader->getLengthField().get() >> 4));
        stream.writeUint4((uint8_t)(ofdmPhyHeader->getLengthField().get() >> 0));
        stream.writeBit(false);
        stream.writeBitRepeatedly(false, 6);
        stream.writeUint16Be(0);
    }
    else {
        // TODO:
        stream.writeByteRepeatedly('?', B(phyHeader->getChunkLength()).get());
    }
}

const Ptr<Chunk> Ieee80211PhyHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    if (true) {
        auto ofdmPhyHeader = makeShared<Ieee80211OfdmPhyHeader>();
        ofdmPhyHeader->setRate(stream.readUint4());
        stream.readBit();
        uint16_t lengthField = 0;
        lengthField |= ((uint16_t)stream.readUint4()) << 8;
        lengthField |= ((uint16_t)stream.readUint4()) << 4;
        lengthField |= ((uint16_t)stream.readUint4()) << 0;
        ofdmPhyHeader->setLengthField(B(lengthField));
        stream.readBit();
        stream.readBitRepeatedly(false, 6);
        stream.readUint16Be();
        return ofdmPhyHeader;
    }
    else {
        auto phyHeader = makeShared<Ieee80211PhyHeader>();
        // TODO:
        phyHeader->setChunkLength(b(192));
        stream.readByteRepeatedly('?', B(phyHeader->getChunkLength()).get());
        return phyHeader;
    }
}

} // namespace physicallayer

} // namespace inet

