//
// Copyright (C) 2018 OpenSim Ltd.
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
// @author: Zoltan Bojthe
//


#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/chunk/BitCountChunk.h"
#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211PhyHeader_m.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211PhyProtocolDissector.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211Tag_m.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::ieee80211Phy, Ieee80211PhyProtocolDissector);

void Ieee80211PhyProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    callback.startProtocolDataUnit(&Protocol::ieee80211Phy);
    auto originalBackOffset = packet->getBackOffset();
    auto payloadEndOffset = packet->getFrontOffset();
    const auto& header = packet->popAtFront<inet::physicallayer::Ieee80211PhyHeader>();
    callback.visitChunk(header, &Protocol::ieee80211Phy);
    payloadEndOffset += header->getChunkLength() + B(header->getLengthField());
    bool incorrect = (payloadEndOffset > originalBackOffset || header->getLengthField() < header->getChunkLength());
    if (incorrect) {
        callback.markIncorrect();
        payloadEndOffset = originalBackOffset;
    }
    packet->setBackOffset(payloadEndOffset);
    callback.dissectPacket(packet, &Protocol::ieee80211Mac);
    packet->setBackOffset(originalBackOffset);
    auto paddingLength = packet->getDataLength();
    if (paddingLength > b(0)) {
        const auto& padding = packet->popAtFront(paddingLength);
        callback.visitChunk(padding, &Protocol::ieee80211Phy);
    }
    callback.endProtocolDataUnit(&Protocol::ieee80211Phy);
}

} // namespace inet

