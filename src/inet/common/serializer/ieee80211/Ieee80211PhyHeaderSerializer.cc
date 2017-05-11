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
#include "inet/common/serializer/ieee80211/Ieee80211PhyHeaderSerializer.h"
#include "inet/common/serializer/ieee80211/Ieee80211PLCPHeaders.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211PhyHeader_m.h"

namespace inet {

namespace serializer {

using namespace physicallayer;

Register_Serializer(Ieee80211PhyHeader, Ieee80211PhyHeaderSerializer);
Register_Serializer(Ieee80211OfdmPhyHeader, Ieee80211PhyHeaderSerializer);

void Ieee80211PhyHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<Chunk>& chunk) const
{
    const auto& phyHeader = std::static_pointer_cast<const Ieee80211PhyHeader>(chunk);
    if (auto ofdmPhyHeader = std::dynamic_pointer_cast<Ieee80211OfdmPhyHeader>(chunk)) {
        Ieee80211OFDMPLCPHeader phyhdr;
        phyhdr.length = ofdmPhyHeader->getLengthField(); // Byte length of the payload
        phyhdr.rate = ofdmPhyHeader->getRate();
        phyhdr.parity = 0; // TODO What is the correct value for this? Check the reference.
        phyhdr.reserved = 0;
        phyhdr.service = 0;
        phyhdr.tail = 0;
        stream.writeBytes((uint8_t *)&phyhdr, byte(OFDM_PLCP_HEADER_LENGTH));
    }
    else {
        // TODO:
        stream.writeByteRepeatedly('?', byte(phyHeader->getChunkLength()).get());
    }
}

Ptr<Chunk> Ieee80211PhyHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    if (true) {
        uint8_t buffer[OFDM_PLCP_HEADER_LENGTH];
        stream.readBytes(buffer, byte(OFDM_PLCP_HEADER_LENGTH));
        auto ofdmPhyHeader = std::make_shared<Ieee80211OfdmPhyHeader>();
        const struct Ieee80211OFDMPLCPHeader& phyhdr = *static_cast<const struct Ieee80211OFDMPLCPHeader *>((void *)&buffer);
        ofdmPhyHeader->setLengthField(phyhdr.length);
        ofdmPhyHeader->setRate(phyhdr.rate);
        return ofdmPhyHeader;
    }
    else {
        auto phyHeader = std::make_shared<Ieee80211PhyHeader>();
        // TODO:
        phyHeader->setChunkLength(bit(192));
        stream.readByteRepeatedly('?', byte(phyHeader->getChunkLength()).get());
        return phyHeader;
    }
}

} // namespace serializer

} // namespace inet

