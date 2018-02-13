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

#include "inet/common/packet/dissector/ProtocolDissector.h"
#include "inet/common/packet/dissector/PacketDissector.h"

#include "inet/linklayer/ethernet/EtherFrame_m.h"
#include "inet/linklayer/ethernet/EtherPhyFrame_m.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee8022/Ieee8022Llc.h"
#include "inet/linklayer/ieee8022/Ieee8022LlcHeader_m.h"
#include "inet/networklayer/arp/ipv4/ArpPacket_m.h"
#include "inet/networklayer/ipv4/IcmpHeader.h"
#include "inet/networklayer/ipv4/Ipv4Header.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211PhyHeader_m.h"
#include "inet/transportlayer/udp/UdpHeader_m.h"

namespace inet {

Register_Protocol_Dissector(nullptr, DefaultDissector);
Register_Protocol_Dissector(&Protocol::ethernet, EthernetDissector);
Register_Protocol_Dissector(&Protocol::ieee80211, Ieee80211Dissector);
Register_Protocol_Dissector(&Protocol::ieee8022, Ieee802LlcDissector);
Register_Protocol_Dissector(&Protocol::arp, ArpDissector);
Register_Protocol_Dissector(&Protocol::ipv4, Ipv4Dissector);
Register_Protocol_Dissector(&Protocol::icmpv4, IcmpDissector);
Register_Protocol_Dissector(&Protocol::udp, UdpDissector);

void DefaultDissector::dissect(Packet *packet, const PacketDissector& packetDissector) const
{
    const auto& header = packet->peekHeader();
    // TODO: use signal tag as opposed to protocol tag?! e.g. IEEE 802. 11 signal -> means 802.11 PHY header
    if (const auto& ethernetPhyHeader = dynamicPtrCast<const inet::EthernetPhyHeader>(header)) {
        packet->setHeaderPopOffset(packet->getHeaderPopOffset() + ethernetPhyHeader->getChunkLength());
        packetDissector.visitChunk(header, nullptr);
        packetDissector.dissectPacket(packet, &Protocol::ethernet);
    }
    else if (const auto& ieeeIeee80211PhyHeader = dynamicPtrCast<const inet::physicallayer::Ieee80211PhyHeader>(header)) {
        packet->setHeaderPopOffset(packet->getHeaderPopOffset() + ieeeIeee80211PhyHeader->getChunkLength());
        packetDissector.visitChunk(header, nullptr);
        packetDissector.dissectPacket(packet, &Protocol::ieee80211);
    }
    else
        packetDissector.visitChunk(packet->peekData(), nullptr);
}

void EthernetDissector::dissect(Packet *packet, const PacketDissector& packetDissector) const
{
    const auto& header = packet->popHeader<EthernetMacHeader>();
    packetDissector.visitChunk(header, &Protocol::ethernet);
    auto protocol = nullptr; // TODO: EtherMac::getProtocol(header);
    packetDissector.dissectPacket(packet, protocol);
    const auto& fcs = packet->popTrailer<EthernetFcs>();
    packetDissector.visitChunk(fcs, &Protocol::ethernet);
    auto paddingLength = packet->getDataLength();
    const auto& padding = packet->popTrailer<EthernetPadding>(paddingLength);
    packetDissector.visitChunk(padding, &Protocol::ethernet);
}

void Ieee80211Dissector::dissect(Packet *packet, const PacketDissector& packetDissector) const
{
    const auto& header = packet->popHeader<inet::ieee80211::Ieee80211MacHeader>();
    const auto& trailer = packet->popTrailer<inet::ieee80211::Ieee80211MacTrailer>();
    packetDissector.visitChunk(header, &Protocol::ieee80211);
    // TODO: fragmentation & aggregation
    if (dynamicPtrCast<const inet::ieee80211::Ieee80211DataOrMgmtHeader>(header))
        packetDissector.dissectPacket(packet, &Protocol::ieee8022); // TODO:
    packetDissector.visitChunk(trailer, &Protocol::ieee80211);
}

void Ieee802LlcDissector::dissect(Packet *packet, const PacketDissector& packetDissector) const
{
    const auto& header = packet->popHeader<inet::Ieee8022LlcHeader>();
    packetDissector.visitChunk(header, &Protocol::ieee8022);
    auto protocol = Ieee8022Llc::getProtocol(header);
    packetDissector.dissectPacket(packet, protocol);
}

void ArpDissector::dissect(Packet *packet, const PacketDissector& packetDissector) const
{
    const auto& arpPacket = packet->peekData<ArpPacket>();
    packetDissector.visitChunk(arpPacket, &Protocol::arp);
}

void Ipv4Dissector::dissect(Packet *packet, const PacketDissector& packetDissector) const
{
    const auto& header = packet->popHeader<Ipv4Header>();
    packetDissector.visitChunk(header, &Protocol::ipv4);
    // TODO: fragmentation
//    auto trailerPopOffset = packet->getTrailerPopOffset();
//    auto xxx = packet->getHeaderPopOffset() + B(header->getTotalLengthField());
//    packet->setTrailerPopOffset(xxx);
    packetDissector.dissectPacket(packet, header->getProtocol());
//    assert(packet->getDataLength() == B(0));
//    packet->setHeaderPopOffset(xxx);
//    packet->setTrailerPopOffset(trailerPopOffset);
}

void IcmpDissector::dissect(Packet *packet, const PacketDissector& packetDissector) const
{
    const auto& header = packet->popHeader<IcmpHeader>();
    packetDissector.visitChunk(header, &Protocol::icmpv4);
    packetDissector.dissectPacket(packet, nullptr);
}

void UdpDissector::dissect(Packet *packet, const PacketDissector& packetDissector) const
{
    const auto& header = packet->popHeader<UdpHeader>();
    packetDissector.visitChunk(header, &Protocol::udp);
    packetDissector.dissectPacket(packet, nullptr);
}

} // namespace

