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

#include "inet/common/INETDefs.h"

#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/L3Address.h"

#ifdef WITH_ETHERNET
#include "inet/linklayer/ethernet/EtherFrame_m.h"
#endif // ifdef WITH_ETHERNET


#ifdef WITH_IPv4
#include "inet/networklayer/arp/ipv4/ArpPacket_m.h"
#include "inet/networklayer/ipv4/IcmpHeader.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#endif // ifdef WITH_IPv4

#ifdef WITH_TCP_COMMON
#include "inet/transportlayer/tcp_common/TcpHeader.h"
#endif // ifdef WITH_TCP_COMMON

#ifdef WITH_UDP
#include "inet/transportlayer/udp/UdpHeader_m.h"
#endif // ifdef WITH_UDP

#ifdef WITH_IEEE80211
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#endif // ifdef WITH_IEEE80211

#include "inet/networklayer/contract/NetworkHeaderBase_m.h"

#ifdef WITH_RIP
#include "inet/routing/rip/RipPacket_m.h"
#endif // ifdef WITH_RIP

#ifdef WITH_RADIO
#include "inet/physicallayer/common/packetlevel/Signal.h"
#include "inet/physicallayer/analogmodel/packetlevel/ScalarTransmission.h"
#endif // ifdef WITH_RADIO

namespace inet {

#ifdef WITH_RADIO
using namespace physicallayer;
#endif // ifdef WITH_RADIO

class INET_API InetPacketPrinter2 : public cMessagePrinter
{
  protected:
    mutable L3Address srcAddr;
    mutable L3Address destAddr;

  protected:
#ifdef WITH_IPv4
    std::string formatARPPacket(const ArpPacket *packet) const;
    std::string formatICMPPacket(const IcmpHeader *packet) const;
#endif // ifdef WITH_IPv4
#ifdef WITH_IEEE80211
    std::string formatIeee80211Frame(const ieee80211::Ieee80211MacHeader *packet) const;
#endif // ifdef WITH_IEEE80211
#ifdef WITH_RIP
    std::string formatRIPPacket(const RipPacket *packet) const;
#endif // ifdef WITH_RIP
#ifdef WITH_RADIO
    std::string formatSignal(const Signal *packet) const;
#endif // ifdef WITH_RADIO
#ifdef WITH_TCP_COMMON
    std::string formatTCPPacket(const tcp::TcpHeader *tcpSeg) const;
#endif // ifdef WITH_TCP_COMMON
#ifdef WITH_UDP
    std::string formatUDPPacket(const UdpHeader *udpPacket) const;
#endif // ifdef WITH_UDP
    std::string formatPacket(Packet *packet) const;

  public:
    InetPacketPrinter2() {}
    virtual ~InetPacketPrinter2() {}
    virtual int getScoreFor(cMessage *msg) const override;
    virtual void printMessage(std::ostream& os, cMessage *msg) const override;
};

Register_MessagePrinter(InetPacketPrinter2);

static const char INFO_SEPAR[] = "  \t";
//static const char INFO_SEPAR[] = "   ";

int InetPacketPrinter2::getScoreFor(cMessage *msg) const
{
    return msg->isPacket() ? 21 : 0;
}

void InetPacketPrinter2::printMessage(std::ostream& os, cMessage *msg) const
{
    std::string outs;

    //reset mutable variables
    srcAddr.reset();
    destAddr.reset();

    const char *separ = "";
    for (cPacket *pk = dynamic_cast<cPacket *>(msg); pk; pk = pk->getEncapsulatedPacket()) {
        std::ostringstream out;
        if (Packet *pck = dynamic_cast<Packet*>(pk)) {
            out << formatPacket(pck);
        }
        else
            out << separ << pk->getClassName() << ":" << pk->getByteLength() << " bytes";
        out << separ << outs;
        outs = out.str();
        separ = INFO_SEPAR;
    }
    os << outs;
}

std::string InetPacketPrinter2::formatPacket(Packet *pk) const
{
    std::string outs;

    //reset mutable variables
    srcAddr.reset();
    destAddr.reset();

    std::ostringstream out;
    const char *separ = "";
    auto packet = new Packet(pk->getName(), pk->peekData());
    while (auto chunkref = packet->popHeader(b(-1), Chunk::PF_ALLOW_NULLPTR)) {
        const auto chunk = chunkref.get();
        std::ostringstream out;

        //TODO slicechunk???

        if (const NetworkHeaderBase *l3Header = dynamic_cast<const NetworkHeaderBase *>(chunk)) {
            srcAddr = l3Header->getSourceAddress();
            destAddr = l3Header->getDestinationAddress();
#ifdef WITH_IPv4
            if (const auto *ipv4Header = dynamic_cast<const Ipv4Header *>(chunk)) {
                out << "Ipv4: " << srcAddr << " > " << destAddr;
                if (ipv4Header->getMoreFragments() || ipv4Header->getFragmentOffset() > 0) {
                    out << " " << (ipv4Header->getMoreFragments() ? "" : "last ")
                        << "fragment with offset=" << ipv4Header->getFragmentOffset() << " of ";
                }
            }
            else
#endif // ifdef WITH_IPv4
                out << chunk->getClassName() << ": " << srcAddr << " > " << destAddr;
        }
#ifdef WITH_ETHERNET
        else if (const auto eth = dynamic_cast<const EthernetMacHeader *>(chunk)) {
            out << "ETH: " << eth->getSrc() << " > " << eth->getDest();
            if (const auto tc = packet->peekTrailer(b(-1), Chunk::PF_ALLOW_NULLPTR).get())
                if (typeid(*tc) == typeid(EthernetFcs)) {
                    const auto& fcs = packet->popTrailer<EthernetFcs>();
                    (void)fcs;    //TODO do we show the FCS?
                }
            //FIXME llc/qtag/snap/...
        }
#endif // ifdef WITH_ETHERNET
#ifdef WITH_TCP_COMMON
        else if (const auto tcpHeader = dynamic_cast<const tcp::TcpHeader *>(chunk)) {
            out << formatTCPPacket(tcpHeader);
        }
#endif // ifdef WITH_TCP_COMMON
#ifdef WITH_UDP
        else if (const auto udpHeader = dynamic_cast<const UdpHeader *>(chunk)) {
            out << formatUDPPacket(udpHeader);
        }
#endif // ifdef WITH_UDP
#ifdef WITH_IPv4
        else if (const auto ipv4Header = dynamic_cast<const IcmpHeader *>(chunk)) {
            out << formatICMPPacket(ipv4Header);
        }
        else if (const auto arp = dynamic_cast<const ArpPacket *>(chunk)) {
            out << formatARPPacket(arp);
        }
#endif // ifdef WITH_IPv4
#ifdef WITH_IEEE80211
        else if (const auto ieee80211Hdr = dynamic_cast<const ieee80211::Ieee80211MacHeader *>(chunk)) {
            out << formatIeee80211Frame(ieee80211Hdr);
        }
#endif // ifdef WITH_IEEE80211
#ifdef WITH_RIP
        else if (const auto rip = dynamic_cast<const RipPacket *>(chunk)) {
            out << formatRIPPacket(rip);
        }
#endif // ifdef WITH_RIP
#ifdef WITH_RADIO
        else if (const auto signal = dynamic_cast<const Signal *>(chunk)) {
            out << formatSignal(signal);
        }
#endif // ifdef WITH_RADIO
        else
            out << chunk->getClassName() << ":" << chunk->getChunkLength();
        out << separ << outs;
        outs = out.str();
        separ = INFO_SEPAR;
    }
    delete packet;
    return outs;
}

#ifdef WITH_IPv4
std::string InetPacketPrinter2::formatARPPacket(const ArpPacket *packet) const
{
    std::ostringstream os;
    switch (packet->getOpcode()) {
        case ARP_REQUEST:
            os << "ARP req: " << packet->getDestIPAddress()
               << "=? (s=" << packet->getSrcIPAddress() << "(" << packet->getSrcMACAddress() << "))";
            break;

        case ARP_REPLY:
            os << "ARP reply: "
               << packet->getSrcIPAddress() << "=" << packet->getSrcMACAddress()
               << " (d=" << packet->getDestIPAddress() << "(" << packet->getDestMACAddress() << "))"
            ;
            break;

        case ARP_RARP_REQUEST:
            os << "RARP req: " << packet->getDestMACAddress()
               << "=? (s=" << packet->getSrcIPAddress() << "(" << packet->getSrcMACAddress() << "))";
            break;

        case ARP_RARP_REPLY:
            os << "RARP reply: "
               << packet->getSrcMACAddress() << "=" << packet->getSrcIPAddress()
               << " (d=" << packet->getDestIPAddress() << "(" << packet->getDestMACAddress() << "))";
            break;

        default:
            os << "ARP op=" << packet->getOpcode() << ": d=" << packet->getDestIPAddress()
               << "(" << packet->getDestMACAddress()
               << ") s=" << packet->getSrcIPAddress()
               << "(" << packet->getSrcMACAddress() << ")";
            break;
    }
    return os.str();
}
#endif // ifdef WITH_IPv4

#ifdef WITH_IEEE80211
std::string InetPacketPrinter2::formatIeee80211Frame(const ieee80211::Ieee80211MacHeader *packet) const
{
    using namespace ieee80211;

    std::ostringstream os;
    os << "WLAN ";
    switch (packet->getType()) {
        case ST_ASSOCIATIONREQUEST:
            os << " assoc req";    //TODO
            break;

        case ST_ASSOCIATIONRESPONSE:
            os << " assoc resp";    //TODO
            break;

        case ST_REASSOCIATIONREQUEST:
            os << " reassoc req";    //TODO
            break;

        case ST_REASSOCIATIONRESPONSE:
            os << " reassoc resp";    //TODO
            break;

        case ST_PROBEREQUEST:
            os << " probe request";    //TODO
            break;

        case ST_PROBERESPONSE:
            os << " probe response";    //TODO
            break;

        case ST_BEACON:
            os << "beacon";    //TODO
            break;

        case ST_ATIM:
            os << " atim";    //TODO
            break;

        case ST_DISASSOCIATION:
            os << " disassoc";    //TODO
            break;

        case ST_AUTHENTICATION:
            os << " auth";    //TODO
            break;

        case ST_DEAUTHENTICATION:
            os << " deauth";    //TODO
            break;

        case ST_ACTION:
            os << " action";    //TODO
            break;

        case ST_NOACKACTION:
            os << " noackaction";    //TODO
            break;

        case ST_PSPOLL:
            os << " pspoll";    //TODO
            break;

        case ST_RTS: {
            const Ieee80211RtsFrame *pk = check_and_cast<const Ieee80211RtsFrame *>(packet);
            os << " rts " << pk->getTransmitterAddress() << " to " << packet->getReceiverAddress();
            break;
        }

        case ST_CTS:
            os << " cts " << packet->getReceiverAddress();
            break;

        case ST_ACK:
            os << " ack " << packet->getReceiverAddress();
            break;

        case ST_BLOCKACK_REQ:
            os << " reassoc resp";    //TODO
            break;

        case ST_BLOCKACK:
            os << " block ack";    //TODO
            break;

        case ST_DATA:
        case ST_DATA_WITH_QOS:
            os << " data";    //TODO
            break;

        case ST_LBMS_REQUEST:
            os << " lbms req";    //TODO
            break;

        case ST_LBMS_REPORT:
            os << " lbms report";    //TODO
            break;

        default:
            os << "??? (" << packet->getClassName() << ")";
            break;
    }
    return os.str();
}
#endif // ifdef WITH_IEEE80211

#ifdef WITH_TCP_COMMON
std::string InetPacketPrinter2::formatTCPPacket(const tcp::TcpHeader *tcpSeg) const
{
    std::ostringstream os;
    os << "TCP: " << srcAddr << '.' << tcpSeg->getSrcPort() << " > " << destAddr << '.' << tcpSeg->getDestPort() << ":";
    // flags
    bool flags = false;
    if (tcpSeg->getUrgBit()) {
        flags = true;
        os << " U";
    }
    if (tcpSeg->getAckBit()) {
        flags = true;
        os << " A";
    }
    if (tcpSeg->getPshBit()) {
        flags = true;
        os << " P";
    }
    if (tcpSeg->getRstBit()) {
        flags = true;
        os << " R";
    }
    if (tcpSeg->getSynBit()) {
        flags = true;
        os << " S";
    }
    if (tcpSeg->getFinBit()) {
        flags = true;
        os << " F";
    }
    if (!flags) {
        os << " .";
    }

    // data-seqno
    os << " seq " << tcpSeg->getSequenceNo();

    // ack
    if (tcpSeg->getAckBit())
        os << " ack " << tcpSeg->getAckNo();

    // window
    os << " win " << tcpSeg->getWindow();

    // urgent
    if (tcpSeg->getUrgBit())
        os << " urg " << tcpSeg->getUrgentPointer();
    return os.str();
}
#endif // ifdef WITH_TCP_COMMON

#ifdef WITH_UDP
std::string InetPacketPrinter2::formatUDPPacket(const UdpHeader *udpPacket) const
{
    std::ostringstream os;
    os << "UDP: " << srcAddr << '.' << udpPacket->getSourcePort() << " > " << destAddr << '.' << udpPacket->getDestinationPort()
       << ": (" << udpPacket->getTotalLengthField() << " bytes)";
    return os.str();
}
#endif // ifdef WITH_UDP

//std::string InetPacketPrinter2::formatPingPayload(const PingPayload *packet) const
//{
//    std::ostringstream os;
//    os << "PING ";
//#ifdef WITH_IPv4
//    IcmpHeader *owner = dynamic_cast<IcmpHeader *>(packet->getOwner());
//    if (owner) {
//        switch (owner->getType()) {
//            case ICMP_ECHO_REQUEST:
//                os << "req ";
//                break;
//
//            case ICMP_ECHO_REPLY:
//                os << "reply ";
//                break;
//
//            default:
//                break;
//        }
//    }
//#endif // ifdef WITH_IPv4
//    os << srcAddr << " to " << destAddr
//       << " (" << packet->getByteLength() << " bytes) id=" << packet->getId()
//       << " seq=" << packet->getSeqNo();
//
//    return os.str();
//}

#ifdef WITH_IPv4
std::string InetPacketPrinter2::formatICMPPacket(const IcmpHeader *icmpHeader) const
{
    std::ostringstream os;
    switch (icmpHeader->getType()) {
        case ICMP_ECHO_REQUEST:
            os << "ICMP echo request " << srcAddr << " to " << destAddr;
            if (auto echo = dynamic_cast<const IcmpEchoRequest *>(icmpHeader))
                os << " id=" << echo->getIdentifier() << ", seq=" << echo->getSeqNumber();
            break;

        case ICMP_ECHO_REPLY:
            os << "ICMP echo reply " << srcAddr << " to " << destAddr;
            if (auto echo = dynamic_cast<const IcmpEchoReply *>(icmpHeader))
                os << " id=" << echo->getIdentifier() << ", seq=" << echo->getSeqNumber();
            break;

        case ICMP_DESTINATION_UNREACHABLE:
            os << "ICMP dest unreachable " << srcAddr << " to " << destAddr << " type=" << icmpHeader->getType() << " code=" << icmpHeader->getCode()
               << " origin:" << INFO_SEPAR;
//FIXME ICMP payload was showed on right side of ICMP header
//            auto subPk = new Packet("", pk->popHeader(byte(icmpHeader->get)))
//            InetPacketPrinter2().printMessage(os, subPk);
//            showEncapsulatedPackets = false;    // stop printing
            break;

        default:
            os << "ICMP " << srcAddr << " to " << destAddr << " type=" << icmpHeader->getType() << " code=" << icmpHeader->getCode();
            break;
    }
    return os.str();
}
#endif // ifdef WITH_IPv4

#ifdef WITH_RIP
std::string InetPacketPrinter2::formatRIPPacket(const RipPacket *packet) const
{
    std::ostringstream os;
    os << "RIP: ";
    switch (packet->getCommand()) {
        case RIP_REQUEST:
            os << "req ";
            break;

        case RIP_RESPONSE:
            os << "resp ";
            break;

        default:
            os << "unknown ";
            break;
    }
    unsigned int size = packet->getEntryArraySize();
    for (unsigned int i = 0; i < size; ++i) {
        const RipEntry& entry = packet->getEntry(i);
        if (i > 0)
            os << "; ";
        if (i > 2) {
            os << "...(" << size << " entries)";
            break;
        }
        os << entry.address << "/" << entry.prefixLength;
        if (!entry.nextHop.isUnspecified())
            os << "->" << entry.nextHop;
        if (entry.metric == 16)
            os << " unroutable";
        else
            os << " m=" << entry.metric;
    }
    return os.str();
}
#endif // ifdef WITH_RIP

#ifdef WITH_RADIO
std::string InetPacketPrinter2::formatSignal(const Signal *packet) const
{
    std::ostringstream os;
    // Note: Do NOT try to print transmission's properties here! getTransmission() will likely
    // return an invalid pointer here, because the transmission is no longer kept in the Medium.
    // const ITransmission *transmission = packet->getTransmission();
    os << "duration=" << SIMTIME_DBL(packet->getDuration()) * 1000 << "ms";
    return os.str();
}
#endif // ifdef WITH_RADIO

} // namespace inet

