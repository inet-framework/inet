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
#include "inet/common/packet/chunk/Chunk.h"
#include "inet/common/packet/chunk/SliceChunk.h"
#include "inet/common/packet/Packet.h"

#include "inet/networklayer/common/L3Address.h"

#ifdef WITH_IPv4
#include "inet/networklayer/ipv4/IcmpHeader.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#endif // ifdef WITH_IPv4

#ifdef WITH_TCP_COMMON
#include "inet/transportlayer/tcp_common/TcpHeader.h"
#endif // ifdef WITH_TCP_COMMON

#ifdef WITH_UDP
#include "inet/transportlayer/udp/UdpHeader_m.h"
#endif // ifdef WITH_UDP

namespace inet {

class INET_API InetPacketPrinter : public cMessagePrinter
{
  protected:
#ifdef WITH_TCP_COMMON
    void printTCPPacket(std::ostream& os, L3Address& srcAddr, L3Address& destAddr, Packet *packet, const Ptr<const tcp::TcpHeader>& tcpHeader) const;
#endif // ifdef WITH_TCP_COMMON
#ifdef WITH_UDP
    void printUDPPacket(std::ostream& os, L3Address& srcAddr, L3Address& destAddr, Packet *packet, const Ptr<const UdpHeader>& udpHeader) const;
#endif // ifdef WITH_UDP
#ifdef WITH_IPv4
    void printICMPPacket(std::ostream& os, L3Address& srcAddr, L3Address& destAddr, Packet *packet, const Ptr<const IcmpHeader>& icmpHeader) const;
#endif // ifdef WITH_IPv4
    void printChunk(std::ostream& os, L3Address& srcAddr, L3Address& destAddr, Packet *packet, const Ptr<const Chunk>& chunk) const;

  public:
    InetPacketPrinter() {}
    virtual ~InetPacketPrinter() {}
    virtual int getScoreFor(cMessage *msg) const override;
    virtual void printMessage(std::ostream& os, cMessage *msg) const override;
};

Register_MessagePrinter(InetPacketPrinter);

int InetPacketPrinter::getScoreFor(cMessage *msg) const
{
    return msg->isPacket() ? 20 : 0;
}

void InetPacketPrinter::printMessage(std::ostream& os, cMessage *msg) const
{
    L3Address srcAddr, destAddr;

    const char *separ = "";
    for (cPacket *pk = dynamic_cast<cPacket *>(msg); pk; pk = pk->getEncapsulatedPacket()) {
        os << separ;
        if (Packet *pck = dynamic_cast<Packet *>(pk)) {
            auto packet = new Packet(pck->getName(), pck->peekData());
            while (const auto& chunk = packet->popHeader(b(-1), Chunk::PF_ALLOW_NULLPTR)) {
                if (const auto& sliceChunk = dynamicPtrCast<const SliceChunk>(chunk)) {
                    os << "slice from " << sliceChunk->getOffset() << ", length=" << sliceChunk->getLength();  //FIXME TODO show the sliced chunk
                }
                else
                    printChunk(os, srcAddr, destAddr, packet, chunk);
            }
            delete packet;
        }
        else
            os << pk->getClassName() << ":" << pk->getByteLength() << " bytes";
        separ = "  \t";
    }
    os << separ << "(" << msg->getClassName() << ")" << " id=" << msg->getId() << " kind=" << msg->getKind();
}

void InetPacketPrinter::printChunk(std::ostream& os, L3Address& srcAddr, L3Address& destAddr, Packet *packet, const Ptr<const Chunk>& chunk) const
{
#ifdef WITH_IPv4
    if (const auto& ipv4Header = dynamicPtrCast<const Ipv4Header>(chunk)) {
        if (ipv4Header->getMoreFragments() || ipv4Header->getFragmentOffset() > 0)
            os << (ipv4Header->getMoreFragments() ? "" : "last ")
               << "fragment with offset=" << ipv4Header->getFragmentOffset() << " of ";
    }
    else
#endif    // WITH_IPv4
#ifdef WITH_TCP_COMMON
    if (const auto& tcpHeader = dynamicPtrCast<const tcp::TcpHeader>(chunk)) {
        printTCPPacket(os, srcAddr, destAddr, packet, tcpHeader);
        return;
    }
    else
#endif // ifdef WITH_TCP_COMMON
#ifdef WITH_UDP
    if (const auto& udpHeader = dynamicPtrCast<const UdpHeader>(chunk)) {
        printUDPPacket(os, srcAddr, destAddr, packet, udpHeader);
        return;
    }
    else
#endif // ifdef WITH_UDP
#ifdef WITH_IPv4
    if (const auto &icmpHeader = dynamicPtrCast<const IcmpHeader>(chunk)) {
        printICMPPacket(os, srcAddr, destAddr, packet, icmpHeader);
        return;
    }
    else
#endif // ifdef WITH_IPv4
    {
        os << chunk->getChunkLength() << " " << chunk->getClassName();
    }
}

#ifdef WITH_TCP_COMMON
void InetPacketPrinter::printTCPPacket(std::ostream& os, L3Address& srcAddr, L3Address& destAddr, Packet *packet, const Ptr<const tcp::TcpHeader>& tcpHeader) const
{
    os << " Tcp: " << srcAddr << '.' << tcpHeader->getSrcPort() << " > " << destAddr << '.' << tcpHeader->getDestPort() << ": ";
    // flags
    bool flags = false;
    if (tcpHeader->getUrgBit()) { flags = true; os << "U "; }
    if (tcpHeader->getAckBit()) { flags = true; os << "A "; }
    if (tcpHeader->getPshBit()) { flags = true; os << "P "; }
    if (tcpHeader->getRstBit()) { flags = true; os << "R "; }
    if (tcpHeader->getSynBit()) { flags = true; os << "S "; }
    if (tcpHeader->getFinBit()) { flags = true; os << "F "; }
    if (!flags) { os << ". "; }

    // data-seqno
    os << tcpHeader->getSequenceNo() << " ";

    // ack
    if (tcpHeader->getAckBit())
        os << "ack " << tcpHeader->getAckNo() << " ";

    // window
    os << "win " << tcpHeader->getWindow() << " ";

    // urgent
    if (tcpHeader->getUrgBit())
        os << "urg " << tcpHeader->getUrgentPointer() << " ";
}
#endif // ifdef WITH_TCP_COMMON

#ifdef WITH_UDP
void InetPacketPrinter::printUDPPacket(std::ostream& os, L3Address& srcAddr, L3Address& destAddr, Packet *packet, const Ptr<const UdpHeader>& udpHeader) const
{

    os << " Udp: " << srcAddr << '.' << udpHeader->getSourcePort() << " > " << destAddr << '.' << udpHeader->getDestinationPort()
       << ": (" << udpHeader->getTotalLengthField() << ")";
}
#endif // ifdef WITH_UDP

#ifdef WITH_IPv4
void InetPacketPrinter::printICMPPacket(std::ostream& os, L3Address& srcAddr, L3Address& destAddr, Packet *packet, const Ptr<const IcmpHeader>& icmpHeader) const
{
    switch (icmpHeader->getType()) {
        case ICMP_ECHO_REQUEST: {
            const auto& echoRq = CHK(dynamicPtrCast<const IcmpEchoRequest>(icmpHeader));
            os << "ping " << srcAddr << " to " << destAddr
               << " id=" << echoRq->getIdentifier() << " seq=" << echoRq->getSeqNumber();
            break;
        }

        case ICMP_ECHO_REPLY: {
            const auto& echoReply = CHK(dynamicPtrCast<const IcmpEchoReply>(icmpHeader));
            os << "pong " << srcAddr << " to " << destAddr
               << " id=" << echoReply->getIdentifier() << " seq=" << echoReply->getSeqNumber();
            break;
        }

        case ICMP_DESTINATION_UNREACHABLE:
            os << "ICMP dest unreachable " << srcAddr << " to " << destAddr << " type=" << icmpHeader->getType() << " code=" << icmpHeader->getCode()
               << " origin: ";
            // printMessage(os, icmpHeader->getEncapsulatedPacket());
            break;

        default:
            os << "ICMP " << srcAddr << " to " << destAddr << " type=" << icmpHeader->getType() << " code=" << icmpHeader->getCode();
            break;
    }
}
#endif // ifdef WITH_IPv4

} // namespace inet

