//
// Copyright (C) 2005 Michael Tuexen
// Copyright (C) 2008 Irene Ruengeler
// Copyright (C) 2009 Thomas Dreibholz
// Copyright (C) 2009 Thomas Reschka
// Copyright (C) 2011 Zoltan Bojthe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include <errno.h>

#include "inet/common/packet/Packet.h"
#include "inet/common/packet/recorder/PacketDump.h"

#ifdef WITH_IPv4
#include "inet/networklayer/arp/ipv4/ArpPacket_m.h"
#include "inet/networklayer/ipv4/IcmpHeader.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#endif // ifdef WITH_IPv4

#ifdef WITH_IPv6
#include "inet/networklayer/ipv6/Ipv6Header.h"
#endif // ifdef WITH_IPv6

#ifdef WITH_SCTP
#include "inet/transportlayer/sctp/SctpAssociation.h"
#include "inet/transportlayer/sctp/SctpHeader.h"
#endif // ifdef WITH_SCTP

#ifdef WITH_TCP_COMMON
#include "inet/transportlayer/tcp_common/TcpHeader.h"
#endif // ifdef WITH_TCP_COMMON

#ifdef WITH_UDP
#include "inet/transportlayer/udp/UdpHeader_m.h"
#endif // ifdef WITH_UDP


namespace inet {

PacketDump::PacketDump()
{
    outp = &std::cout;
    verbose = false;
}

PacketDump::~PacketDump()
{
}

#ifdef WITH_SCTP
void PacketDump::sctpDump(const char *label, Packet * pk, const Ptr<const sctp::SctpHeader>& sctpmsg,
        const std::string& srcAddr, const std::string& destAddr, const char *comment)
{
    using namespace sctp;

    std::ostream& out = *outp;

    // seq and time (not part of the tcpdump format)
    char buf[30];
    sprintf(buf, "[%.3f%s] ", simTime().dbl(), label);
    out << buf;

    out << "[sctp] " << srcAddr << " > " << destAddr;
    uint32 numberOfChunks;
    const SctpChunk *chunk;
    uint8 type;
    // src/dest
    out << srcAddr << "." << sctpmsg->getSrcPort() << " > "
        << destAddr << "." << sctpmsg->getDestPort() << ": ";

    numberOfChunks = sctpmsg->getSctpChunksArraySize();
    out << "numberOfChunks=" << numberOfChunks << " VTag=" << sctpmsg->getVTag() << "\n";

    if (pk->hasBitError())
        out << "Packet has bit error!!\n";

    for (uint32 i = 0; i < numberOfChunks; i++) {
        chunk = sctpmsg->getSctpChunks(i);
        type = chunk->getSctpChunkType();

        // FIXME create a getChunkTypeName(SctpChunkType x) function in SCTP code and use it!
        switch (type) {
            case INIT:
                out << "INIT ";
                break;

            case INIT_ACK:
                out << "INIT_ACK ";
                break;

            case COOKIE_ECHO:
                out << "COOKIE_ECHO ";
                break;

            case COOKIE_ACK:
                out << "COOKIE_ACK ";
                break;

            case DATA:
                out << "DATA ";
                break;

            case SACK:
                out << "SACK ";
                break;

            case HEARTBEAT:
                out << "HEARTBEAT ";
                break;

            case HEARTBEAT_ACK:
                out << "HEARTBEAT_ACK ";
                break;

            case ABORT:
                out << "ABORT ";
                break;

            case SHUTDOWN:
                out << "SHUTDOWN ";
                break;

            case SHUTDOWN_ACK:
                out << "SHUTDOWN_ACK ";
                break;

            case SHUTDOWN_COMPLETE:
                out << "SHUTDOWN_COMPLETE ";
                break;

            case ERRORTYPE:
                out << "ERROR";
                break;
        }
    }

    if (verbose) {
        out << endl;

        for (uint32 i = 0; i < numberOfChunks; i++) {
            chunk = sctpmsg->getSctpChunks(i);
            type = chunk->getSctpChunkType();

            sprintf(buf, "   %3u: ", i + 1);
            out << buf;

            switch (type) {
                case INIT: {
                    const SctpInitChunk *initChunk;
                    initChunk = check_and_cast<const SctpInitChunk *>(chunk);
                    out << "INIT[InitiateTag=";
                    out << initChunk->getInitTag();
                    out << "; a_rwnd=";
                    out << initChunk->getA_rwnd();
                    out << "; OS=";
                    out << initChunk->getNoOutStreams();
                    out << "; IS=";
                    out << initChunk->getNoInStreams();
                    out << "; InitialTSN=";
                    out << initChunk->getInitTsn();

                    if (initChunk->getAddressesArraySize() > 0) {
                        out << "; Addresses=";

                        for (uint32 i = 0; i < initChunk->getAddressesArraySize(); i++) {
                            if (i > 0)
                                out << ",";

                            if (initChunk->getAddresses(i).getType() == L3Address::IPv6)
                                out << initChunk->getAddresses(i).str();
                            else
                                out << initChunk->getAddresses(i);
                        }
                    }

                    out << "]";
                    break;
                }

                case INIT_ACK: {
                    const SctpInitAckChunk *initackChunk;
                    initackChunk = check_and_cast<const SctpInitAckChunk *>(chunk);
                    out << "INIT_ACK[InitiateTag=";
                    out << initackChunk->getInitTag();
                    out << "; a_rwnd=";
                    out << initackChunk->getA_rwnd();
                    out << "; OS=";
                    out << initackChunk->getNoOutStreams();
                    out << "; IS=";
                    out << initackChunk->getNoInStreams();
                    out << "; InitialTSN=";
                    out << initackChunk->getInitTsn();
                    out << "; CookieLength=";
                    out << initackChunk->getCookieArraySize();

                    if (initackChunk->getAddressesArraySize() > 0) {
                        out << "; Addresses=";

                        for (uint32 i = 0; i < initackChunk->getAddressesArraySize(); i++) {
                            if (i > 0)
                                out << ",";

                            out << initackChunk->getAddresses(i);
                        }
                    }

                    out << "]";
                    break;
                }

                case COOKIE_ECHO:
                    out << "COOKIE_ECHO[CookieLength=";
                    out << chunk->getLength() - 4;
                    out << "]";
                    break;

                case COOKIE_ACK:
                    out << "COOKIE_ACK ";
                    break;

                case DATA: {
                    const SctpDataChunk *dataChunk;
                    dataChunk = check_and_cast<const SctpDataChunk *>(chunk);
                    out << "DATA[TSN=";
                    out << dataChunk->getTsn();
                    out << "; SID=";
                    out << dataChunk->getSid();
                    out << "; SSN=";
                    out << dataChunk->getSsn();
                    out << "; PPID=";
                    out << dataChunk->getPpid();
                    out << "; PayloadLength=";
                    out << dataChunk->getLength() - 16;
                    out << "]";
                    break;
                }

                case SACK: {
                    const SctpSackChunk *sackChunk;
                    sackChunk = check_and_cast<const SctpSackChunk *>(chunk);
                    out << "SACK[CumTSNAck=";
                    out << sackChunk->getCumTsnAck();
                    out << "; a_rwnd=";
                    out << sackChunk->getA_rwnd();

                    if (sackChunk->getGapStartArraySize() > 0) {
                        out << "; Gaps=";

                        for (uint32 i = 0; i < sackChunk->getGapStartArraySize(); i++) {
                            if (i > 0)
                                out << ", ";

                            out << sackChunk->getGapStart(i) << "-" << sackChunk->getGapStop(i);
                        }
                    }

                    if (sackChunk->getDupTsnsArraySize() > 0) {
                        out << "; Dups=";

                        for (uint32 i = 0; i < sackChunk->getDupTsnsArraySize(); i++) {
                            if (i > 0)
                                out << ", ";

                            out << sackChunk->getDupTsns(i);
                        }
                    }

                    out << "]";
                    break;
                }

                case HEARTBEAT:
                    const SctpHeartbeatChunk *heartbeatChunk;
                    heartbeatChunk = check_and_cast<const SctpHeartbeatChunk *>(chunk);
                    out << "HEARTBEAT[InfoLength=";
                    out << chunk->getLength() - 4;
                    out << "; time=";
                    out << heartbeatChunk->getTimeField();
                    out << "]";
                    break;

                case HEARTBEAT_ACK:
                    out << "HEARTBEAT_ACK[InfoLength=";
                    out << chunk->getLength() - 4;
                    out << "]";
                    break;

                case ABORT:
                    const SctpAbortChunk *abortChunk;
                    abortChunk = check_and_cast<const SctpAbortChunk *>(chunk);
                    out << "ABORT[T-Bit=";
                    out << abortChunk->getT_Bit();
                    out << "]";
                    break;

                case SHUTDOWN:
                    const SctpShutdownChunk *shutdown;
                    shutdown = check_and_cast<const SctpShutdownChunk *>(chunk);
                    out << "SHUTDOWN[CumTSNAck=";
                    out << shutdown->getCumTsnAck();
                    out << "]";
                    break;

                case SHUTDOWN_ACK:
                    out << "SHUTDOWN_ACK ";
                    break;

                case SHUTDOWN_COMPLETE:
                    out << "SHUTDOWN_COMPLETE ";
                    break;

                case ERRORTYPE: {
                    out << "ERRORTYPE ";
                    const SctpErrorChunk *errorChunk;
                    errorChunk = check_and_cast<const SctpErrorChunk *>(chunk);
                    uint32 numberOfParameters = errorChunk->getParametersArraySize();
                    uint32 parameterType;

                    for (uint32 i = 0; i < numberOfParameters; i++) {
                        const SctpParameter *param = errorChunk->getParameters(i);
                        parameterType = param->getParameterType();
                        out << parameterType << " ";
                    }
                    break;
                }
            }
            out << endl;
        }
    }

    // comment
    if (comment)
        out << "# " << comment;

    out << endl;
}
#endif // ifndef WITH_SCTP

void PacketDump::dump(const char *label, const char *msg)
{
    std::ostream& out = *outp;

    // seq and time (not part of the tcpdump format)
    char buf[30];

    sprintf(buf, "[%.3f%s] ", simTime().dbl(), label);
    out << buf << msg << endl;
}

void PacketDump::dumpPacket(bool l2r, const cPacket *msg)
{
    std::ostream& out = *outp;
    auto packet = dynamic_cast<const Packet *>(msg);
    if (packet == nullptr)
        return;

    std::string leftAddr = "A";
    std::string rightAddr = "B";
    auto packetCopy = packet->dup();
    while(const auto& chunk = packetCopy->popAtFront(b(-1), Chunk::PF_ALLOW_NULLPTR)) {
#ifdef WITH_IPv4
        if (const auto& ipv4Hdr = dynamicPtrCast<const Ipv4Header>(chunk)) {
            leftAddr = ipv4Hdr->getSourceAddress().str();
            rightAddr = ipv4Hdr->getDestinationAddress().str();
            dumpIpv4(l2r, "", ipv4Hdr, "");
        }
        else
        if (const auto& arpPacket = dynamicPtrCast<const ArpPacket>(chunk)) {
            dumpArp(l2r, "", arpPacket, "");
        }
        else
        if (dynamicPtrCast<const IcmpHeader>(chunk)) {
            out << "ICMPMessage " << packet->getName() << (packet->hasBitError() ? " (BitError)" : "") << endl;
        }
        else
#endif // ifdef WITH_IPv4
#ifdef WITH_IPv6
        if (const auto& ipv6Hdr = dynamicPtrCast<const Ipv6Header>(chunk)) {
            dumpIpv6(l2r, "", ipv6Hdr);
        }
        else
#endif // ifdef WITH_IPv6
#ifdef WITH_SCTP
        if (const auto& sctpMessage = dynamicPtrCast<const sctp::SctpHeader>(chunk)) {
            sctpDump("", packetCopy, sctpMessage, std::string(l2r ? leftAddr : rightAddr), std::string(l2r ?  rightAddr: leftAddr));
        }
        else
#endif // ifdef WITH_SCTP
#ifdef WITH_TCP_COMMON
        if (const auto& tcpHdr = dynamicPtrCast<const tcp::TcpHeader>(chunk)) {
            tcpDump(l2r, "", tcpHdr, msg->getByteLength(), (l2r ? leftAddr : rightAddr), (l2r ? rightAddr : leftAddr));
        }
        else
#endif // ifdef WITH_TCP_COMMON
        {
            out << chunk->str();
        }
    }
    delete packetCopy;
}

#ifdef WITH_UDP
void PacketDump::udpDump(bool l2r, const char *label, const Ptr<const UdpHeader>& udpHeader,
        const std::string& srcAddr, const std::string& destAddr, const char *comment)
{
    std::ostream& out = *outp;

    char buf[30];
    sprintf(buf, "[%.3f%s] ", simTime().dbl(), label);
    out << buf;

      // seq and time (not part of the tcpdump format)
      // src/dest
    if (l2r) {
        out << srcAddr << "." << udpHeader->getSourcePort() << " > ";
        out << destAddr << "." << udpHeader->getDestinationPort() << ": ";
    }
    else {
        out << destAddr << "." << udpHeader->getDestinationPort() << " < ";
        out << srcAddr << "." << udpHeader->getSourcePort() << ": ";
    }

    //out << endl;
    out << "UDP: Payload length=" << udpHeader->getTotalLengthField() - UDP_HEADER_LENGTH << endl;

    // comment
    if (comment)
        out << "# " << comment;

    out << endl;
}
#endif // ifdef WITH_UDP

#ifdef WITH_IPv4
void PacketDump::dumpArp(bool l2r, const char *label, const Ptr<const ArpPacket>& arp, const char *comment)
{
    std::ostream& out = *outp;
    char buf[30];
    sprintf(buf, "[%.3f%s] ", simTime().dbl(), label);
    out << buf << " src: " << arp->getSrcIpAddress() << ", " << arp->getSrcMacAddress()
        << "; dest: " << arp->getDestIpAddress() << ", " << arp->getDestMacAddress() << endl;
}

void PacketDump::dumpIpv4(bool l2r, const char *label, const Ptr<const Ipv4Header>& ipv4Header, const  char *comment)
{
    std::ostream& out = *outp;
    char buf[30];
    std::string classes;

    // some other packet, dump what we can
    // seq and time (not part of the tcpdump format)
    sprintf(buf, "[%.3f%s] ", SIMTIME_DBL(simTime()), label);
    out << buf;
    out << "[IPv4] " << ipv4Header->getSrcAddress() << " > " << ipv4Header->getDestAddress();

    if (ipv4Header->getMoreFragments() || ipv4Header->getFragmentOffset())
        out << ((ipv4Header->getMoreFragments()) ? " inner" : " last") << " fragment from offset " << ipv4Header->getFragmentOffset();

    // comment
    if (comment)
        out << " # " << comment;

    out << endl;

    sprintf(buf, "[%.3f%s] ", SIMTIME_DBL(simTime()), label);
    out << buf << "[IPv4]";

    // comment
    if (comment)
        out << " # " << comment;

    out << endl;
}
#endif // ifdef WITH_IPv4

#ifdef WITH_IPv6
void PacketDump::dumpIpv6(bool l2r, const char *label, const Ptr<const Ipv6Header>& ipv6Header, const char *comment)
{
    using namespace tcp;

    std::ostream& out = *outp;
    char buf[30];

    sprintf(buf, "[%.3f%s] ", SIMTIME_DBL(simTime()), label);
    out << buf;

    out << ipv6Header->str();

    // comment
    if (comment)
        out << " # " << comment;

    out << endl;
}
#endif // ifdef WITH_IPv6

#ifdef WITH_TCP_COMMON
void PacketDump::tcpDump(bool l2r, const char *label, const Ptr<const tcp::TcpHeader>& tcpHeader, int tcpLength,
        const std::string& srcAddr, const std::string& destAddr, const char *comment)
{
    using namespace tcp;

    std::ostream& out = *outp;

    // seq and time (not part of the tcpdump format)
    char buf[30];
    sprintf(buf, "[%.3f%s] ", SIMTIME_DBL(simTime()), label);
    out << buf;

    // src/dest ports
    if (l2r) {
        out << srcAddr << "." << tcpHeader->getSrcPort() << " > ";
        out << destAddr << "." << tcpHeader->getDestPort() << ": ";
    }
    else {
        out << destAddr << "." << tcpHeader->getDestPort() << " < ";
        out << srcAddr << "." << tcpHeader->getSrcPort() << ": ";
    }

    int payloadLength = tcpLength - B(tcpHeader->getHeaderLength()).get();

    // flags
    bool flags = false;
    if (tcpHeader->getUrgBit()) {
        flags = true;
        out << "U ";
    }
    if (tcpHeader->getAckBit()) {
        flags = true;
        out << "A ";
    }
    if (tcpHeader->getPshBit()) {
        flags = true;
        out << "P ";
    }
    if (tcpHeader->getRstBit()) {
        flags = true;
        out << "R ";
    }
    if (tcpHeader->getSynBit()) {
        flags = true;
        out << "S ";
    }
    if (tcpHeader->getFinBit()) {
        flags = true;
        out << "F ";
    }
    if (!flags) {
        out << ". ";
    }

    // data-seqno
    if (payloadLength > 0 || tcpHeader->getSynBit()) {
        out << tcpHeader->getSequenceNo() << ":" << tcpHeader->getSequenceNo() + payloadLength;
        out << "(" << payloadLength << ") ";
    }

    // ack
    if (tcpHeader->getAckBit())
        out << "ack " << tcpHeader->getAckNo() << " ";

    // window
    out << "win " << tcpHeader->getWindow() << " ";

    // urgent
    if (tcpHeader->getUrgBit())
        out << "urg " << tcpHeader->getUrgentPointer() << " ";

    // options present?
    if (tcpHeader->getHeaderLength() > TCP_MIN_HEADER_LENGTH) {
        if (verbose) {
            unsigned short numOptions = tcpHeader->getHeaderOptionArraySize();
            out << "  Option(s):";

            for (int i = 0; i < numOptions; i++) {
                const TcpOption *option = tcpHeader->getHeaderOption(i);
                switch (option->getKind()) {
                    case TCPOPTION_END_OF_OPTION_LIST:
                        break;
                    case TCPOPTION_NO_OPERATION:
                        out << " NOP";
                        break;
                    case TCPOPTION_MAXIMUM_SEGMENT_SIZE:
                        out << " MaxSegSize";
                        break;
                    case TCPOPTION_WINDOW_SCALE:
                        out << " WinScale";
                        break;
                    case TCPOPTION_SACK_PERMITTED:
                        out << " SackPermitted";
                        break;
                    case TCPOPTION_SACK: {
                        auto sackOpt = check_and_cast<const TcpOptionSack *>(option);
                        out << " SACK";
                        for (size_t k = 0; k < sackOpt->getSackItemArraySize(); k++) {
                            const auto& sackItem = sackOpt->getSackItem(k);
                            out << "[" << sackItem.getStart() << "," << sackItem.getEnd() << ")";
                        }
                        break;
                    }
                    case TCPOPTION_TIMESTAMP: {
                        auto tsOpt = check_and_cast<const TcpOptionTimestamp *>(option);
                        out << " TS(" << tsOpt->getSenderTimestamp() << "," << tsOpt->getEchoedTimestamp() << ")";
                        break;
                    }
                    default:
                        out << " (kind=" << option->getKind() << " length=" << option->getLength() << ")"; break;
                }
            }
        }
    }

    // comment
    if (comment)
        out << "# " << comment;

    out << endl;
}
#endif // ifdef WITH_TCP_COMMON

} // namespace inet

