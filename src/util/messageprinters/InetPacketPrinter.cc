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
#include "PingPayload_m.h"

#ifdef WITH_IPv4
#include "ICMPMessage.h"
#include "IPv4Datagram.h"
#else
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

//TODO HACK, remove next line
#include "cmessageprinter.h"

class INET_API InetPacketPrinter : public cMessagePrinter
{
  protected:
    void printTCPPacket(std::ostream& os, IPvXAddress srcAddr, IPvXAddress destAddr, TCPSegment *tcpSeg) const;
    void printUDPPacket(std::ostream& os, IPvXAddress srcAddr, IPvXAddress destAddr, UDPPacket *udpPacket) const;
    void printICMPPacket(std::ostream& os, IPvXAddress srcAddr, IPvXAddress destAddr, ICMPMessage *packet) const;

  public:
    InetPacketPrinter() {}
    virtual ~InetPacketPrinter() {}
    virtual int getScoreFor(cMessage *msg) const;
    virtual void printMessage(std::ostream& os, cMessage *msg) const;
};

Register_MessagePrinter(InetPacketPrinter);

int InetPacketPrinter::getScoreFor(cMessage *msg) const
{
    return msg->isPacket() ? 20 : 0;
}

void InetPacketPrinter::printMessage(std::ostream& os, cMessage *msg) const
{
    IPvXAddress srcAddr, destAddr;

    for (cPacket *pk = dynamic_cast<cPacket *>(msg); pk; pk = pk->getEncapsulatedPacket()) {
        if (false) {
        }
#ifdef WITH_IPv4
        else if (dynamic_cast<IPv4Datagram *>(pk)) {
            IPv4Datagram *ipv4dgram = static_cast<IPv4Datagram *>(pk);
            srcAddr = ipv4dgram->getSrcAddress();
            destAddr = ipv4dgram->getDestAddress();
            if (ipv4dgram->getMoreFragments() || ipv4dgram->getFragmentOffset() > 0)
                os << (ipv4dgram->getMoreFragments() ? "" : "last ")
                   << "fragment with offset=" << ipv4dgram->getFragmentOffset() << " of ";
        }
#endif
#ifdef WITH_IPv6
        else if (dynamic_cast<IPv6Datagram *>(pk)) {
            IPv6Datagram *dgram = static_cast<IPv6Datagram *>(pk);
            srcAddr = dgram->getSrcAddress();
            destAddr = dgram->getDestAddress();
        }
#endif
#ifdef WITH_TCP_COMMON
        else if (dynamic_cast<TCPSegment *>(pk)) {
            printTCPPacket(os, srcAddr, destAddr, static_cast<TCPSegment *>(pk));
            return;
        }
#endif
#ifdef WITH_UDP
        else if (dynamic_cast<UDPPacket *>(pk)) {
            printUDPPacket(os, srcAddr, destAddr, static_cast<UDPPacket *>(pk));
            return;
        }
#endif
#ifdef WITH_IPv4
        else if (dynamic_cast<ICMPMessage *>(pk)) {
            printICMPPacket(os, srcAddr, destAddr, static_cast<ICMPMessage *>(pk));
            return;
        }
#endif
    }
    os << "(" << msg->getClassName() << ")" << " id=" << msg->getId() << " kind=" << msg->getKind();
}

void InetPacketPrinter::printTCPPacket(std::ostream& os, IPvXAddress srcAddr, IPvXAddress destAddr, TCPSegment *tcpSeg) const
{
#ifdef WITH_TCP_COMMON
    os << " TCP: " << srcAddr << '.' << tcpSeg->getSrcPort() << " > " << destAddr << '.' << tcpSeg->getDestPort() << ": ";
    // flags
    bool flags = false;
    if (tcpSeg->getUrgBit()) { flags = true; os << "U "; }
    if (tcpSeg->getAckBit()) { flags = true; os << "A "; }
    if (tcpSeg->getPshBit()) { flags = true; os << "P "; }
    if (tcpSeg->getRstBit()) { flags = true; os << "R "; }
    if (tcpSeg->getSynBit()) { flags = true; os << "S "; }
    if (tcpSeg->getFinBit()) { flags = true; os << "F "; }
    if (!flags) { os << ". "; }

    // data-seqno
    if (tcpSeg->getPayloadLength()>0 || tcpSeg->getSynBit()) {
        os << tcpSeg->getSequenceNo() << ":" << tcpSeg->getSequenceNo()+tcpSeg->getPayloadLength();
        os << "(" << tcpSeg->getPayloadLength() << ") ";
    }

    // ack
    if (tcpSeg->getAckBit())
        os << "ack " << tcpSeg->getAckNo() << " ";

    // window
    os << "win " << tcpSeg->getWindow() << " ";

    // urgent
    if (tcpSeg->getUrgBit())
        os << "urg " << tcpSeg->getUrgentPointer() << " ";
#else
    os << " TCP: " << srcAddr << ".? > " << destAddr << ".?";
#endif
}

void InetPacketPrinter::printUDPPacket(std::ostream& os, IPvXAddress srcAddr, IPvXAddress destAddr, UDPPacket *udpPacket) const
{
#ifdef WITH_UDP
    os << " UDP: " << srcAddr << '.' << udpPacket->getSourcePort() << " > " << destAddr << '.' << udpPacket->getDestinationPort()
       << ": (" << udpPacket->getByteLength() << ")";
#else
    os << " UDP: " << srcAddr << ".? > " << destAddr << ".?";
#endif
}

void InetPacketPrinter::printICMPPacket(std::ostream& os, IPvXAddress srcAddr, IPvXAddress destAddr, ICMPMessage *packet) const
{
#ifdef WITH_IPv4
    switch (packet->getType()) {
        case ICMP_ECHO_REQUEST:
        {
            PingPayload *payload = check_and_cast<PingPayload *>(packet->getEncapsulatedPacket());
            os << "ping " << srcAddr << " to " << destAddr
               << " (" << packet->getByteLength() << " bytes) id=" << payload->getId() << " seq=" <<payload->getSeqNo();
            break;
        }
        case ICMP_ECHO_REPLY:
        {
            PingPayload *payload = check_and_cast<PingPayload *>(packet->getEncapsulatedPacket());
            os << "pong " << srcAddr << " to " << destAddr
               << " (" << packet->getByteLength() << " bytes) id=" << payload->getId() << " seq=" <<payload->getSeqNo();
            break;
        }
        case ICMP_DESTINATION_UNREACHABLE:
            os << "ICMP dest unreachable " << srcAddr << " to " << destAddr << " type=" << packet->getType() << " code=" << packet->getCode()
               << " origin: ";
            printMessage(os, packet->getEncapsulatedPacket());
            break;
        default:
            os << "ICMP " << srcAddr << " to " << destAddr << " type=" << packet->getType() << " code=" << packet->getCode();
            break;
    }
#else
    os << " ICMP: " << srcAddr << " > " << destAddr;
#endif
}

#endif // Register_MessagePrinter

