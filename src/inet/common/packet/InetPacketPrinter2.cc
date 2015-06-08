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

#include "inet/networklayer/common/L3Address.h"

#ifdef WITH_ETHERNET
#include "inet/linklayer/ethernet/EtherFrame.h"
#else // ifdef WITH_ETHERNET
namespace inet { class EtherFrame; }
#endif // ifdef WITH_ETHERNET

#ifdef WITH_IPv4
#include "inet/networklayer/arp/ipv4/ARPPacket_m.h"
#include "inet/networklayer/ipv4/ICMPMessage.h"
#include "inet/networklayer/ipv4/IPv4Datagram.h"
#else // ifdef WITH_IPv4

namespace inet {

class ARPPacket;
class ICMPMessage;
class IPv4Datagram;

} // namespace inet

#endif // ifdef WITH_IPv4

#ifdef WITH_TCP_COMMON
#include "inet/transportlayer/tcp_common/TCPSegment.h"
#else // ifdef WITH_TCP_COMMON
namespace inet { namespace tcp { class TCPSegment; } }
#endif // ifdef WITH_TCP_COMMON

#ifdef WITH_UDP
#include "inet/transportlayer/udp/UDPPacket.h"
#else // ifdef WITH_UDP
namespace inet { class UDPPacket; }
#endif // ifdef WITH_UDP

#ifdef WITH_IEEE80211
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#else // ifdef WITH_IEEE80211
namespace inet { namespace ieee80211 { class Ieee80211Frame; } }
#endif // ifdef WITH_IEEE80211

#include "inet/networklayer/contract/INetworkDatagram.h"
#include "inet/applications/pingapp/PingPayload_m.h"

#ifdef WITH_RIP
#include "inet/routing/rip/RIPPacket_m.h"
#else // ifdef WITH_RIP
class RIPPacket;
#endif // ifdef WITH_RIP

#ifdef WITH_RADIO
#include "inet/physicallayer/common/packetlevel/RadioFrame.h"
#include "inet/physicallayer/analogmodel/packetlevel/ScalarTransmission.h"
#else // ifdef WITH_RADIO
namespace inet { namespace physicallayer { class RadioFrame; } }
#endif // ifdef WITH_RADIO

//TODO Do not move next line to top of file - opp_makemake can not detect dependencies inside of '#if' with omnetpp-specific defines
#if OMNETPP_VERSION >= 0x0405

namespace inet {

using namespace physicallayer;

class INET_API InetPacketPrinter2 : public cMessagePrinter
{
  protected:
    mutable bool showEncapsulatedPackets;
    mutable L3Address srcAddr;
    mutable L3Address destAddr;

  protected:
    std::string formatARPPacket(ARPPacket *packet) const;
    std::string formatICMPPacket(ICMPMessage *packet) const;
    std::string formatIeee80211Frame(ieee80211::Ieee80211Frame *packet) const;
    std::string formatPingPayload(PingPayload *packet) const;
    std::string formatRIPPacket(RIPPacket *packet) const;
    std::string formatRadioFrame(RadioFrame *packet) const;
    std::string formatTCPPacket(tcp::TCPSegment *tcpSeg) const;
    std::string formatUDPPacket(UDPPacket *udpPacket) const;

  public:
    InetPacketPrinter2() { showEncapsulatedPackets = true; }
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
    srcAddr = destAddr = L3Address();
    showEncapsulatedPackets = true;

    for (cPacket *pk = dynamic_cast<cPacket *>(msg); showEncapsulatedPackets && pk; pk = pk->getEncapsulatedPacket()) {
        std::ostringstream out;
        INetworkDatagram *dgram = dynamic_cast<INetworkDatagram *>(pk);
        if (dgram) {
            srcAddr = dgram->getSourceAddress();
            destAddr = dgram->getDestinationAddress();
#ifdef WITH_IPv4
            if (dynamic_cast<IPv4Datagram *>(pk)) {
                IPv4Datagram *ipv4dgram = static_cast<IPv4Datagram *>(pk);
                out << "IPv4: " << srcAddr << " > " << destAddr;
                if (ipv4dgram->getMoreFragments() || ipv4dgram->getFragmentOffset() > 0) {
                    out << " " << (ipv4dgram->getMoreFragments() ? "" : "last ")
                        << "fragment with offset=" << ipv4dgram->getFragmentOffset() << " of ";
                }
            }
            else
#endif // ifdef WITH_IPv4
            out << pk->getClassName() << ": " << srcAddr << " > " << destAddr;
        }
#ifdef WITH_ETHERNET
        else if (dynamic_cast<EtherFrame *>(pk)) {
            EtherFrame *eth = static_cast<EtherFrame *>(pk);
            out << "ETH: " << eth->getSrc() << " > " << eth->getDest() << " (" << eth->getByteLength() << " bytes)";
        }
#endif // ifdef WITH_ETHERNET
#ifdef WITH_TCP_COMMON
        else if (dynamic_cast<tcp::TCPSegment *>(pk)) {
            out << formatTCPPacket(static_cast<tcp::TCPSegment *>(pk));
        }
#endif // ifdef WITH_TCP_COMMON
#ifdef WITH_UDP
        else if (dynamic_cast<UDPPacket *>(pk)) {
            out << formatUDPPacket(static_cast<UDPPacket *>(pk));
        }
#endif // ifdef WITH_UDP
#ifdef WITH_IPv4
        else if (dynamic_cast<ICMPMessage *>(pk)) {
            out << formatICMPPacket(static_cast<ICMPMessage *>(pk));
        }
        else if (dynamic_cast<ARPPacket *>(pk)) {
            out << formatARPPacket(static_cast<ARPPacket *>(pk));
        }
#endif // ifdef WITH_IPv4
#ifdef WITH_IEEE80211
        else if (dynamic_cast<ieee80211::Ieee80211Frame *>(pk)) {
            out << formatIeee80211Frame(static_cast<ieee80211::Ieee80211Frame *>(pk));
        }
#endif // ifdef WITH_IEEE80211
        else if (dynamic_cast<PingPayload *>(pk)) {
            out << formatPingPayload(static_cast<PingPayload *>(pk));
        }
#ifdef WITH_RIP
        else if (dynamic_cast<RIPPacket *>(pk)) {
            out << formatRIPPacket(static_cast<RIPPacket *>(pk));
        }
#endif // ifdef WITH_RIP
#ifdef WITH_RADIO
        else if (dynamic_cast<RadioFrame *>(pk)) {
            out << formatRadioFrame(static_cast<RadioFrame *>(pk));
        }
#endif // ifdef WITH_RADIO
        else
            out << pk->getClassName() << ":" << pk->getByteLength() << " bytes";
        if (outs.length())
            out << INFO_SEPAR << outs;
        outs = out.str();
    }
    os << outs;
}

std::string InetPacketPrinter2::formatARPPacket(ARPPacket *packet) const
{
    std::ostringstream os;
#ifdef WITH_IPv4
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
#endif // ifdef WITH_IPv4
    return os.str();
}

std::string InetPacketPrinter2::formatIeee80211Frame(ieee80211::Ieee80211Frame *packet) const
{
    using namespace ieee80211;

    std::ostringstream os;
#ifdef WITH_IEEE80211
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
            Ieee80211RTSFrame *pk = check_and_cast<Ieee80211RTSFrame *>(packet);
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
#endif // ifdef WITH_IEEE80211
    return os.str();
}

std::string InetPacketPrinter2::formatTCPPacket(tcp::TCPSegment *tcpSeg) const
{
    std::ostringstream os;
#ifdef WITH_TCP_COMMON
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
    if (tcpSeg->getPayloadLength() > 0 || tcpSeg->getSynBit()) {
        os << " " << tcpSeg->getSequenceNo() << ":" << tcpSeg->getSequenceNo() + tcpSeg->getPayloadLength();
        os << "(" << tcpSeg->getPayloadLength() << ")";
    }

    // ack
    if (tcpSeg->getAckBit())
        os << " ack " << tcpSeg->getAckNo();

    // window
    os << " win " << tcpSeg->getWindow();

    // urgent
    if (tcpSeg->getUrgBit())
        os << " urg " << tcpSeg->getUrgentPointer();
#endif // ifdef WITH_TCP_COMMON
    return os.str();
}

std::string InetPacketPrinter2::formatUDPPacket(UDPPacket *udpPacket) const
{
    std::ostringstream os;
#ifdef WITH_UDP
    os << "UDP: " << srcAddr << '.' << udpPacket->getSourcePort() << " > " << destAddr << '.' << udpPacket->getDestinationPort()
       << ": (" << udpPacket->getByteLength() << ")";
#endif // ifdef WITH_UDP
    return os.str();
}

std::string InetPacketPrinter2::formatPingPayload(PingPayload *packet) const
{
    std::ostringstream os;
    os << "PING ";
#ifdef WITH_IPv4
    ICMPMessage *owner = dynamic_cast<ICMPMessage *>(packet->getOwner());
    if (owner) {
        switch (owner->getType()) {
            case ICMP_ECHO_REQUEST:
                os << "req ";
                break;

            case ICMP_ECHO_REPLY:
                os << "reply ";
                break;

            default:
                break;
        }
    }
#endif // ifdef WITH_IPv4
    os << srcAddr << " to " << destAddr
       << " (" << packet->getByteLength() << " bytes) id=" << packet->getId()
       << " seq=" << packet->getSeqNo();

    return os.str();
}

std::string InetPacketPrinter2::formatICMPPacket(ICMPMessage *packet) const
{
    std::ostringstream os;
#ifdef WITH_IPv4
    switch (packet->getType()) {
        case ICMP_ECHO_REQUEST:
            os << "ICMP echo request " << srcAddr << " to " << destAddr;
            break;

        case ICMP_ECHO_REPLY:
            os << "ICMP echo reply " << srcAddr << " to " << destAddr;
            break;

        case ICMP_DESTINATION_UNREACHABLE:
            os << "ICMP dest unreachable " << srcAddr << " to " << destAddr << " type=" << packet->getType() << " code=" << packet->getCode()
               << " origin:" << INFO_SEPAR;
            InetPacketPrinter2().printMessage(os, packet->getEncapsulatedPacket());
            showEncapsulatedPackets = false;    // stop printing
            break;

        default:
            os << "ICMP " << srcAddr << " to " << destAddr << " type=" << packet->getType() << " code=" << packet->getCode();
            break;
    }
#endif // ifdef WITH_IPv4
    return os.str();
}

std::string InetPacketPrinter2::formatRIPPacket(RIPPacket *packet) const
{
    std::ostringstream os;
#ifdef WITH_RIP
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
        RIPEntry& entry = packet->getEntry(i);
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
#endif // ifdef WITH_RIP
    return os.str();
}

std::string InetPacketPrinter2::formatRadioFrame(RadioFrame *packet) const
{
    std::ostringstream os;
#ifdef WITH_RADIO
    // Note: Do NOT try to print transmission's properties here! getTransmission() will likely
    // return an invalid pointer here, because the transmission is no longer kept in the Medium.
    // const ITransmission *transmission = packet->getTransmission();
    os << "duration=" << SIMTIME_DBL(packet->getDuration()) * 1000 << "ms";
#endif // ifdef WITH_RADIO
    return os.str();
}

} // namespace inet

#endif    // Register_MessagePrinter

