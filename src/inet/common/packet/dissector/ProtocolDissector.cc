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
#include "inet/linklayer/ethernet/EtherFrame_m.h"
#include "inet/linklayer/ethernet/EtherPhyFrame_m.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee8022/Ieee8022Llc.h"
#include "inet/linklayer/ieee8022/Ieee8022LlcHeader_m.h"
#include "inet/linklayer/ppp/PppFrame_m.h"
#include "inet/networklayer/arp/ipv4/ArpPacket_m.h"
#include "inet/networklayer/ipv4/IcmpHeader.h"
#include "inet/networklayer/ipv4/Ipv4Header.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211PhyHeader_m.h"
#include "inet/transportlayer/tcp_common/TcpHeader.h"
#include "inet/transportlayer/udp/UdpHeader_m.h"

namespace inet {

Register_Protocol_Dissector(nullptr, DefaultDissector);
Register_Protocol_Dissector(&Protocol::ppp, PppDissector);
Register_Protocol_Dissector(&Protocol::ethernet, EthernetDissector);
Register_Protocol_Dissector(&Protocol::ieee80211, Ieee80211Dissector);
Register_Protocol_Dissector(&Protocol::ieee8022, Ieee802LlcDissector);
Register_Protocol_Dissector(&Protocol::arp, ArpDissector);
Register_Protocol_Dissector(&Protocol::ipv4, Ipv4Dissector);
Register_Protocol_Dissector(&Protocol::icmpv4, IcmpDissector);
Register_Protocol_Dissector(&Protocol::udp, UdpDissector);
Register_Protocol_Dissector(&Protocol::tcp, TcpDissector);

void DefaultDissector::dissect(Packet *packet, ICallback& callback) const
{
    const auto& header = packet->peekHeader();
    // TODO: use signal tag as opposed to protocol tag?! e.g. IEEE 802. 11 signal -> means 802.11 PHY header
    if (const auto& ethernetPhyHeader = dynamicPtrCast<const inet::EthernetPhyHeader>(header)) {
        packet->setHeaderPopOffset(packet->getHeaderPopOffset() + ethernetPhyHeader->getChunkLength());
        callback.startProtocol(nullptr);
        callback.visitChunk(header, nullptr);
        callback.dissectPacket(packet, &Protocol::ethernet);
        callback.endProtocol(nullptr);
    }
    else if (const auto& ieeeIeee80211PhyHeader = dynamicPtrCast<const inet::physicallayer::Ieee80211PhyHeader>(header)) {
        callback.startProtocol(nullptr);
        packet->setHeaderPopOffset(packet->getHeaderPopOffset() + ieeeIeee80211PhyHeader->getChunkLength());
        callback.visitChunk(header, nullptr);
        const auto& trailer = packet->peekTrailer();
        // TODO: KLUDGE: padding length
        auto ieeeIeee80211PhyPadding = dynamicPtrCast<const BitCountChunk>(trailer);
        if (ieeeIeee80211PhyPadding != nullptr)
            packet->setTrailerPopOffset(packet->getTrailerPopOffset() - ieeeIeee80211PhyPadding->getChunkLength());
        callback.dissectPacket(packet, &Protocol::ieee80211);
        if (ieeeIeee80211PhyPadding != nullptr)
            callback.visitChunk(ieeeIeee80211PhyPadding, nullptr);
        callback.endProtocol(nullptr);
    }
    else {
        callback.visitChunk(packet->peekData(), nullptr);
        packet->setHeaderPopOffset(packet->getTrailerPopOffset());
    }
}

void PppDissector::dissect(Packet *packet, ICallback& callback) const
{
    callback.startProtocol(&Protocol::ppp);
    const auto& header = packet->popHeader<PppHeader>();
    const auto& trailer = packet->popTrailer<PppTrailer>();
    callback.visitChunk(header, &Protocol::ppp);
    auto payloadProtocol = ProtocolGroup::pppprotocol.getProtocol(header->getProtocol());
    callback.dissectPacket(packet, payloadProtocol);
    callback.visitChunk(trailer, &Protocol::ppp);
    callback.endProtocol(&Protocol::ppp);
}

void EthernetDissector::dissect(Packet *packet, ICallback& callback) const
{
    const auto& header = packet->popHeader<EthernetMacHeader>();
    callback.startProtocol(&Protocol::ethernet);
    callback.visitChunk(header, &Protocol::ethernet);
    const auto& fcs = packet->popTrailer<EthernetFcs>();
    // TODO:
    auto payloadProtocol = ProtocolGroup::ethertype.getProtocol(header->getTypeOrLength());
    callback.dissectPacket(packet, payloadProtocol);
    auto paddingLength = packet->getDataLength();
    if (paddingLength > b(0)) {
        const auto& padding = packet->popTrailer<EthernetPadding>(paddingLength);
        callback.visitChunk(padding, &Protocol::ethernet);
    }
    callback.visitChunk(fcs, &Protocol::ethernet);
    callback.endProtocol(&Protocol::ethernet);
}

void Ieee80211Dissector::dissect(Packet *packet, ICallback& callback) const
{
    const auto& header = packet->popHeader<inet::ieee80211::Ieee80211MacHeader>();
    const auto& trailer = packet->popTrailer<inet::ieee80211::Ieee80211MacTrailer>();
    callback.startProtocol(&Protocol::ieee80211);
    callback.visitChunk(header, &Protocol::ieee80211);
    // TODO: fragmentation & aggregation
    if (auto dataHeader = dynamicPtrCast<const inet::ieee80211::Ieee80211DataHeader>(header)) {
        if (dataHeader->getMoreFragments() || dataHeader->getFragmentNumber() != 0)
            callback.dissectPacket(packet, nullptr);
        else if (dataHeader->getAMsduPresent()) {
            auto originalTrailerPopOffset = packet->getTrailerPopOffset();
            int paddingLength = 0;
            while (packet->getDataLength() > B(0)) {
                packet->setHeaderPopOffset(packet->getHeaderPopOffset() + B(paddingLength == 4 ? 0 : paddingLength));
                const auto& msduSubframeHeader = packet->popHeader<ieee80211::Ieee80211MsduSubframeHeader>();
                auto msduEndOffset = packet->getHeaderPopOffset() + B(msduSubframeHeader->getLength());
                packet->setTrailerPopOffset(msduEndOffset);
                callback.dissectPacket(packet, &Protocol::ieee8022);
                paddingLength = 4 - B(msduSubframeHeader->getChunkLength() + B(msduSubframeHeader->getLength())).get() % 4;
                packet->setTrailerPopOffset(originalTrailerPopOffset);
                packet->setHeaderPopOffset(msduEndOffset);
            }
        }
        else
            callback.dissectPacket(packet, &Protocol::ieee8022);
    }
    else if (dynamicPtrCast<const inet::ieee80211::Ieee80211ActionFrame>(header))
        ASSERT(packet->getDataLength() == b(0));
    else if (dynamicPtrCast<const inet::ieee80211::Ieee80211MgmtHeader>(header))
        callback.visitChunk(packet->peekData(), &Protocol::ieee80211);
    else // TODO: if (dynamicPtrCast<const inet::ieee80211::Ieee80211ControlFrame>(header))
        ASSERT(packet->getDataLength() == b(0));
    callback.visitChunk(trailer, &Protocol::ieee80211);
    callback.endProtocol(&Protocol::ieee80211);
}

void Ieee802LlcDissector::dissect(Packet *packet, ICallback& callback) const
{
    const auto& header = packet->popHeader<inet::Ieee8022LlcHeader>();
    callback.startProtocol(&Protocol::ieee8022);
    callback.visitChunk(header, &Protocol::ieee8022);
    auto protocol = Ieee8022Llc::getProtocol(header);
    callback.dissectPacket(packet, protocol);
    callback.endProtocol(&Protocol::ieee8022);
}

void ArpDissector::dissect(Packet *packet, ICallback& callback) const
{
    const auto& arpPacket = packet->popHeader<ArpPacket>();
    callback.startProtocol(&Protocol::arp);
    callback.visitChunk(arpPacket, &Protocol::arp);
    callback.endProtocol(&Protocol::arp);
}

void Ipv4Dissector::dissect(Packet *packet, ICallback& callback) const
{
    const auto& header = packet->popHeader<Ipv4Header>();
    callback.startProtocol(&Protocol::ipv4);
    callback.visitChunk(header, &Protocol::ipv4);
    // TODO: fragmentation
//    auto trailerPopOffset = packet->getTrailerPopOffset();
//    auto xxx = packet->getHeaderPopOffset() + B(header->getTotalLengthField());
//    packet->setTrailerPopOffset(xxx);
    callback.dissectPacket(packet, header->getProtocol());
//    assert(packet->getDataLength() == B(0));
//    packet->setHeaderPopOffset(xxx);
//    packet->setTrailerPopOffset(trailerPopOffset);
    callback.endProtocol(&Protocol::ipv4);
}

void IcmpDissector::dissect(Packet *packet, ICallback& callback) const
{
    const auto& header = packet->popHeader<IcmpHeader>();
    callback.startProtocol(&Protocol::icmpv4);
    callback.visitChunk(header, &Protocol::icmpv4);
    callback.dissectPacket(packet, nullptr);
    callback.endProtocol(&Protocol::icmpv4);
}

void UdpDissector::dissect(Packet *packet, ICallback& callback) const
{
    const auto& header = packet->popHeader<UdpHeader>();
    callback.startProtocol(&Protocol::udp);
    callback.visitChunk(header, &Protocol::udp);
    callback.dissectPacket(packet, nullptr);
    callback.endProtocol(&Protocol::udp);
}

void TcpDissector::dissect(Packet *packet, ICallback& callback) const
{
    const auto& header = packet->popHeader<tcp::TcpHeader>();
    callback.startProtocol(&Protocol::tcp);
    callback.visitChunk(header, &Protocol::tcp);
    if (packet->getDataLength() != b(0))
        callback.dissectPacket(packet, nullptr);
    callback.endProtocol(&Protocol::tcp);
}

} // namespace

