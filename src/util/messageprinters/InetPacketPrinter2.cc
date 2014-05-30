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


#include "INETDefs.h"

#ifdef Register_MessagePrinter

#include "IPvXAddress.h"

#ifdef WITH_ETHERNET
#include "EtherFrame.h"
#else
class EtherFrame;
#endif

#ifdef WITH_IPv4
#include "ARPPacket_m.h"
#include "ICMPMessage.h"
#include "IPv4Datagram.h"
#else
class ARPPacket;
class ICMPMessage;
class IPv4Datagram;
#endif

#ifdef WITH_IPv6
#include "IPv6Datagram.h"
#else
class IPv6Datagram;
#endif

#ifdef WITH_TCP_COMMON
#include "TCPSegment.h"
#else
class TCPSegment;
#endif

#ifdef WITH_UDP
#include "UDPPacket.h"
#else
class UDPPacket;
#endif

#ifdef WITH_IEEE80211
#include "Ieee80211Frame_m.h"
#else
class Ieee80211Frame;
#endif

#include "PingPayload_m.h"

#ifdef WITH_RIP
#include "RIPPacket_m.h"
#else
class RIPPacket;
#endif

#ifdef WITH_RADIO
#include "AirFrame_m.h"
#else
class AirFrame;
#endif

//TODO HACK, remove next line
#include "cmessageprinter.h"

class INET_API InetPacketPrinter2 : public cMessagePrinter
{
    protected:
        mutable bool showEncapsulatedPackets;
        mutable IPvXAddress srcAddr;
        mutable IPvXAddress destAddr;
    protected:
        std::string formatARPPacket(ARPPacket *packet) const;
        std::string formatICMPPacket(ICMPMessage *packet) const;
        std::string formatIeee80211Frame(Ieee80211Frame *packet) const;
        std::string formatPingPayload(PingPayload *packet) const;
        std::string formatRIPPacket(RIPPacket *packet) const;
        std::string formatAirFrame(AirFrame *packet) const;
        std::string formatTCPPacket(TCPSegment *tcpSeg) const;
        std::string formatUDPPacket(UDPPacket *udpPacket) const;
    public:
        InetPacketPrinter2() { showEncapsulatedPackets = true; }
        virtual ~InetPacketPrinter2() {}
        virtual int getScoreFor(cMessage *msg) const;
        virtual void printMessage(std::ostream& os, cMessage *msg) const;
};

Register_MessagePrinter(InetPacketPrinter2);

//static const char INFO_SEPAR[] = "  \t";
static const char INFO_SEPAR[] = "   ";

int InetPacketPrinter2::getScoreFor(cMessage *msg) const
{
    return msg->isPacket() ? 21 : 0;
}

void InetPacketPrinter2::printMessage(std::ostream& os, cMessage *msg) const
{
    std::string outs;

    //reset mutable variables
    srcAddr = destAddr = IPvXAddress();
    showEncapsulatedPackets = true;

    for (cPacket *pk = dynamic_cast<cPacket *>(msg); showEncapsulatedPackets && pk; pk = pk->getEncapsulatedPacket()) {
        std::ostringstream out;
        if (false) {
        }
#ifdef WITH_IPv4
        else if (dynamic_cast<IPv4Datagram *>(pk)) {
            IPv4Datagram *dgram = dynamic_cast<IPv4Datagram *>(pk);
            srcAddr = dgram->getSrcAddress();
            destAddr = dgram->getDestAddress();
            out << "IPv4: " << srcAddr << " > " << destAddr;
            if (dgram->getMoreFragments() || dgram->getFragmentOffset() > 0) {
                out << " " << (dgram->getMoreFragments() ? "" : "last ")
                    << "fragment with offset=" << dgram->getFragmentOffset() << " of ";
            }
        }
#endif
#ifdef WITH_ETHERNET
        else if (dynamic_cast<EtherFrame *>(pk)) {
            EtherFrame *eth = static_cast<EtherFrame *>(pk);
            out << "ETH: " << eth->getSrc() << " > " << eth->getDest() << " (" << eth->getByteLength() << " bytes)";
        }
#endif
#ifdef WITH_TCP_COMMON
        else if (dynamic_cast<TCPSegment *>(pk)) {
            out << formatTCPPacket(static_cast<TCPSegment *>(pk));
        }
#endif
#ifdef WITH_UDP
        else if (dynamic_cast<UDPPacket *>(pk)) {
            out << formatUDPPacket(static_cast<UDPPacket *>(pk));
        }
#endif
#ifdef WITH_IPv4
        else if (dynamic_cast<ICMPMessage *>(pk)) {
            out << formatICMPPacket(static_cast<ICMPMessage *>(pk));
        }
        else if (dynamic_cast<ARPPacket *>(pk)) {
            out << formatARPPacket(static_cast<ARPPacket *>(pk));
        }
#endif
#ifdef WITH_IEEE80211
        else if (dynamic_cast<Ieee80211Frame *>(pk)) {
            out << formatIeee80211Frame(static_cast<Ieee80211Frame *>(pk));
        }
#endif
        else if (dynamic_cast<PingPayload *>(pk)) {
            out << formatPingPayload(static_cast<PingPayload *>(pk));
        }
#ifdef WITH_RIP
        else if (dynamic_cast<RIPPacket *>(pk)) {
            out << formatRIPPacket(static_cast<RIPPacket *>(pk));
        }
#endif
#ifdef WITH_RADIO
        else if (dynamic_cast<AirFrame *>(pk)) {
            out << formatAirFrame(static_cast<AirFrame *>(pk));
        }
#endif
        else
            out << pk->getClassName() <<":" << pk->getByteLength() << " bytes";
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
#endif
    return os.str();
}

std::string InetPacketPrinter2::formatIeee80211Frame(Ieee80211Frame *packet) const
{
    std::ostringstream os;
#ifdef WITH_IEEE80211
    os << "WLAN ";
    switch (packet->getType()) {
        case ST_ASSOCIATIONREQUEST:
            os << " assoc req";     //TODO
            break;
        case ST_ASSOCIATIONRESPONSE:
            os << " assoc resp";     //TODO
            break;
        case ST_REASSOCIATIONREQUEST:
            os << " reassoc req";     //TODO
            break;
        case ST_REASSOCIATIONRESPONSE:
            os << " reassoc resp";     //TODO
            break;
        case ST_PROBEREQUEST:
            os << " probe request";     //TODO
            break;
        case ST_PROBERESPONSE:
            os << " probe response";     //TODO
            break;
        case ST_BEACON:
            os << "beacon";     //TODO
            break;
        case ST_ATIM:
            os << " atim";     //TODO
            break;
        case ST_DISASSOCIATION:
            os << " disassoc";     //TODO
            break;
        case ST_AUTHENTICATION:
            os << " auth";     //TODO
            break;
        case ST_DEAUTHENTICATION:
            os << " deauth";     //TODO
            break;
        case ST_ACTION:
            os << " action";     //TODO
            break;
        case ST_NOACKACTION:
            os << " noackaction";     //TODO
            break;
        case ST_PSPOLL:
            os << " pspoll";     //TODO
            break;
        case ST_RTS:
        {
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
            os << " reassoc resp";     //TODO
            break;
        case ST_BLOCKACK:
            os << " block ack";     //TODO
            break;
        case ST_DATA:
            os << " data";     //TODO
            break;
        case ST_LBMS_REQUEST:
            os << " lbms req";     //TODO
            break;
        case ST_LBMS_REPORT:
            os << " lbms report";     //TODO
            break;
        default:
            os << "??? (" << packet->getClassName() << ")";
            break;
    }
#endif
    return os.str();
}

std::string InetPacketPrinter2::formatTCPPacket(TCPSegment *tcpSeg) const
{
    std::ostringstream os;
#ifdef WITH_TCP_COMMON
    os << "TCP: " << srcAddr << '.' << tcpSeg->getSrcPort() << " > " << destAddr << '.' << tcpSeg->getDestPort() << ":";
    // flags
    bool flags = false;
    if (tcpSeg->getUrgBit()) { flags = true; os << " U"; }
    if (tcpSeg->getAckBit()) { flags = true; os << " A"; }
    if (tcpSeg->getPshBit()) { flags = true; os << " P"; }
    if (tcpSeg->getRstBit()) { flags = true; os << " R"; }
    if (tcpSeg->getSynBit()) { flags = true; os << " S"; }
    if (tcpSeg->getFinBit()) { flags = true; os << " F"; }
    if (!flags) { os << " ."; }

    // data-seqno
    if (tcpSeg->getPayloadLength()>0 || tcpSeg->getSynBit())
    {
        os << " " << tcpSeg->getSequenceNo() << ":" << tcpSeg->getSequenceNo()+tcpSeg->getPayloadLength();
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
#endif
    return os.str();
}

std::string InetPacketPrinter2::formatUDPPacket(UDPPacket *udpPacket) const
{
    std::ostringstream os;
#ifdef WITH_UDP
    os << "UDP: " << srcAddr << '.' << udpPacket->getSourcePort() << " > " << destAddr << '.' << udpPacket->getDestinationPort()
       << ": (" << udpPacket->getByteLength() << ")";
#endif
    return os.str();
}

std::string InetPacketPrinter2::formatPingPayload(PingPayload *packet) const
{
    std::ostringstream os;
    os << "PING ";
#ifdef WITH_IPv4
    ICMPMessage *owner = dynamic_cast<ICMPMessage *>(packet->getOwner());
    if (owner) {
        switch(owner->getType()) {
            case ICMP_ECHO_REQUEST: os << "req "; break;
            case ICMP_ECHO_REPLY: os << "reply "; break;
            default: break;
        }
    }
#endif
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
            showEncapsulatedPackets = false; // stop printing
            break;
        default:
            os << "ICMP " << srcAddr << " to " << destAddr << " type=" << packet->getType() << " code=" << packet->getCode();
            break;
    }
#endif
    return os.str();
}

std::string InetPacketPrinter2::formatRIPPacket(RIPPacket *packet) const
{
    std::ostringstream os;
#ifdef WITH_RIP
    os << "RIP: ";
    switch(packet->getCommand()) {
        case RIP_REQUEST:  os << "req "; break;
        case RIP_RESPONSE: os << "resp "; break;
        default: os << "unknown "; break;
    }
    unsigned int size = packet->getEntryArraySize();
    for (unsigned int i = 0; i < size; ++i) {
        RIPEntry &entry = packet->getEntry(i);
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
#endif
    return os.str();
}

std::string InetPacketPrinter2::formatAirFrame(AirFrame *packet) const
{
    std::ostringstream os;
#ifdef WITH_RADIO
    os << "RADIO from " << packet->getSenderPos() << " on " << packet->getCarrierFrequency()/1e6
       << "MHz, ch=" << packet->getChannelNumber() << ", duration=" << SIMTIME_DBL(packet->getDuration())*1000 << "ms";
#endif
    return os.str();
}

#endif // Register_MessagePrinter

