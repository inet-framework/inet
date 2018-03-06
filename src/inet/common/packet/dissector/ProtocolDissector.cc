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

#include "inet/common/packet/chunk/BitCountChunk.h"
#include "inet/common/packet/dissector/ProtocolDissector.h"
#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"

// TODO: move individual dissectors into their respective protocol folders
#include "inet/linklayer/ieee8022/Ieee8022Llc.h"
#include "inet/linklayer/ieee8022/Ieee8022LlcHeader_m.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211PhyHeader_m.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211Tag_m.h"
#include "inet/transportlayer/tcp_common/TcpHeader.h"

namespace inet {

Register_Protocol_Dissector(nullptr, DefaultDissector);
Register_Protocol_Dissector(&Protocol::ieee80211Phy, Ieee80211PhyDissector);
Register_Protocol_Dissector(&Protocol::ieee8022, Ieee802LlcDissector);
Register_Protocol_Dissector(&Protocol::tcp, TcpDissector);

void DefaultDissector::dissect(Packet *packet, ICallback& callback) const
{
    callback.startProtocolDataUnit(nullptr);
    callback.visitChunk(packet->peekData(), nullptr);
    packet->setHeaderPopOffset(packet->getTrailerPopOffset());
    callback.endProtocolDataUnit(nullptr);
}

void Ieee80211PhyDissector::dissect(Packet *packet, ICallback& callback) const
{
    const auto& header = packet->popHeader<inet::physicallayer::Ieee80211PhyHeader>();
    callback.startProtocolDataUnit(&Protocol::ieee80211Phy);
    const auto& trailer = packet->peekTrailer();
    // TODO: KLUDGE: padding length
    auto ieee80211PhyPadding = dynamicPtrCast<const BitCountChunk>(trailer);
    if (ieee80211PhyPadding != nullptr)
        packet->setTrailerPopOffset(packet->getTrailerPopOffset() - ieee80211PhyPadding->getChunkLength());
    callback.visitChunk(header, &Protocol::ieee80211Phy);
    callback.dissectPacket(packet, &Protocol::ieee80211Mac);
    if (ieee80211PhyPadding != nullptr)
        callback.visitChunk(ieee80211PhyPadding, nullptr);
    callback.endProtocolDataUnit(&Protocol::ieee80211Phy);
}

void Ieee802LlcDissector::dissect(Packet *packet, ICallback& callback) const
{
    const auto& header = packet->popHeader<inet::Ieee8022LlcHeader>();
    callback.startProtocolDataUnit(&Protocol::ieee8022);
    callback.visitChunk(header, &Protocol::ieee8022);
    auto protocol = Ieee8022Llc::getProtocol(header);
    callback.dissectPacket(packet, protocol);
    callback.endProtocolDataUnit(&Protocol::ieee8022);
}

void TcpDissector::dissect(Packet *packet, ICallback& callback) const
{
    const auto& header = packet->popHeader<tcp::TcpHeader>();
    callback.startProtocolDataUnit(&Protocol::tcp);
    callback.visitChunk(header, &Protocol::tcp);
    if (packet->getDataLength() != b(0))
        callback.dissectPacket(packet, nullptr);
    callback.endProtocolDataUnit(&Protocol::tcp);
}

} // namespace

