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

#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/ieee80211/llc/Ieee80211EtherTypeHeader_m.h"
#include "inet/linklayer/ieee80211/llc/LlcProtocolTag_m.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211MacProtocolDissector.h"
#include "inet/linklayer/ieee8022/Ieee8022LlcHeader_m.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211Tag_m.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::ieee80211Mac, Ieee80211MacProtocolDissector);

const Protocol *Ieee80211MacProtocolDissector::computeLlcProtocol(Packet *packet) const
{
    if (const auto llcTag = packet->findTag<ieee80211::LlcProtocolTag>())
        return llcTag->getProtocol();
    else if (const auto channelTag = packet->findTag<physicallayer::Ieee80211ChannelInd>()) {
        // EtherType protocol discrimination is mandatory for deployments in the 5.9 GHz band
        if (channelTag->getChannel()->getBand() == &physicallayer::Ieee80211CompliantBands::band5_9GHz)
            return &Protocol::ieee80211EtherType;
    }
    const auto& header = packet->peekAtFront();
    if (dynamicPtrCast<const Ieee8022LlcHeader>(header) != nullptr)
        return &Protocol::ieee8022;
    else if (dynamicPtrCast<const Ieee80211EtherTypeHeader>(header) != nullptr)
        return &Protocol::ieee80211EtherType;
    else
        return nullptr;
}

void Ieee80211MacProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    const auto& header = packet->popAtFront<inet::ieee80211::Ieee80211MacHeader>();
    const auto& trailer = packet->popAtBack<inet::ieee80211::Ieee80211MacTrailer>(B(4));
    callback.startProtocolDataUnit(&Protocol::ieee80211Mac);
    callback.visitChunk(header, &Protocol::ieee80211Mac);
    // TODO: fragmentation & aggregation
    if (auto dataHeader = dynamicPtrCast<const inet::ieee80211::Ieee80211DataHeader>(header)) {
        if (dataHeader->getMoreFragments() || dataHeader->getFragmentNumber() != 0)
            callback.dissectPacket(packet, nullptr);
        else if (dataHeader->getAMsduPresent()) {
            auto originalTrailerPopOffset = packet->getBackOffset();
            int paddingLength = 0;
            while (packet->getDataLength() > B(0)) {
                packet->setFrontOffset(packet->getFrontOffset() + B(paddingLength == 4 ? 0 : paddingLength));
                const auto& msduSubframeHeader = packet->popAtFront<ieee80211::Ieee80211MsduSubframeHeader>();
                auto msduEndOffset = packet->getFrontOffset() + B(msduSubframeHeader->getLength());
                packet->setBackOffset(msduEndOffset);
                callback.dissectPacket(packet, computeLlcProtocol(packet));
                paddingLength = 4 - B(msduSubframeHeader->getChunkLength() + B(msduSubframeHeader->getLength())).get() % 4;
                packet->setBackOffset(originalTrailerPopOffset);
                packet->setFrontOffset(msduEndOffset);
            }
        }
        else
            callback.dissectPacket(packet, computeLlcProtocol(packet));
    }
    else if (dynamicPtrCast<const inet::ieee80211::Ieee80211ActionFrame>(header))
        ASSERT(packet->getDataLength() == b(0));
    else if (dynamicPtrCast<const inet::ieee80211::Ieee80211MgmtHeader>(header))
        callback.dissectPacket(packet, &Protocol::ieee80211Mgmt);
    // TODO: else if (dynamicPtrCast<const inet::ieee80211::Ieee80211ControlFrame>(header))
    else
        ASSERT(packet->getDataLength() == b(0));
    callback.visitChunk(trailer, &Protocol::ieee80211Mac);
    callback.endProtocolDataUnit(&Protocol::ieee80211Mac);
}

} // namespace inet

