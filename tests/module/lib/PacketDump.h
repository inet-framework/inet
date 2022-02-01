//
// Copyright (C) 2005 Michael Tuexen
// Copyright (C) 2008 Irene Ruengeler
// Copyright (C) 2009 Thomas Dreibholz
// Copyright (C) 2011 Zoltan Bojthe
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_PACKETDUMP_H
#define __INET_PACKETDUMP_H

#include "inet/common/packet/Packet.h"

namespace inet {

// Foreign declarations:
class Ipv4Header;
class Ipv6Header;
namespace tcp { class TcpHeader; }
class UdpHeader;
class ArpPacket;
namespace sctp { class SctpHeader; }

/**
 * Utility class that provides tcpdump-like functionality. It prints
 * information about each packet on the given output stream.
 */
class INET_API PacketDump
{
  protected:
    bool verbose;
    std::ostream *outp;

  public:
    /**
     * Constructor. The output stream initially points to the C++ standard
     * output (std::cout); you probably want to call
     * <code>setOutStream(getEnvir()->getOStream())</code> to redirect it to EV.
     */
    PacketDump();

    /**
     * Destructor. It does not close the output stream.
     */
    ~PacketDump();

    /**
     * Sets the output stream.
     */
    void setOutStream(std::ostream& o) { outp = &o; }

    /**
     * Returns the output stream.
     */
    std::ostream& getOutStream() const { return *outp; }

    /**
     * Enable/disable verbose output.
     */
    void setVerbose(bool verb) { verbose = verb; }

    /**
     * Returns the verbosity flag.
     */
    bool isVerbose() const { return verbose; }

    /**
     * Writes the given text on the output stream.
     */
    void dump(const char *label, const char *msg);

    /**
     * Dumps info about the given packet. It dispatches to the more specific
     * dump functions. The l2r parameter denotes the direction of the packet.
     */
    void dumpPacket(bool l2r, const cPacket *packet);

#ifdef INET_WITH_IPv4
    /**
     * Dumps info about the given Ipv4 datagram. The l2r parameter denotes the
     * direction of the packet.
     */
    void dumpIpv4(bool l2r, const char *label, const Ptr<const Ipv4Header>& ipv4Header, const char *comment = nullptr);

    void dumpArp(bool l2r, const char *label, const Ptr<const ArpPacket>& arp, const char *comment = nullptr);
#endif // ifdef INET_WITH_IPv4

#ifdef INET_WITH_IPv6
    /**
     * Dumps info about the given Ipv6 datagram. The l2r parameter denotes
     * the direction of the packet.
     */
    void dumpIpv6(bool l2r, const char *label, const Ptr<const Ipv6Header>& ipv6Header, const char *comment = nullptr);
#endif // ifdef INET_WITH_IPv6

#ifdef INET_WITH_SCTP
    /**
     * Dumps info about the given SCTP message.
     */
     void sctpDump(const char *label, Packet * pk, const Ptr<const sctp::SctpHeader>& sctpmsg,
        const std::string& srcAddr, const std::string& destAddr, const char *comment = nullptr);

#endif // ifdef INET_WITH_SCTP

#ifdef INET_WITH_TCP_COMMON
    /**
     * Dumps info about the given TCP segment.
     */
    void tcpDump(bool l2r, const char *label, const Ptr<const tcp::TcpHeader>& tcpHeader, int tcpLength, const std::string& srcAddr,
            const std::string& destAddr, const char *comment = nullptr);
#endif // ifdef INET_WITH_TCP_COMMON

#ifdef INET_WITH_UDP
    /**
     * Dumps info about the given UDP packet.
     */
    void udpDump(bool l2r, const char *label, const Ptr<const UdpHeader>& udpHeader, const std::string& srcAddr,
            const std::string& destAddr, const char *comment);
#endif // ifdef INET_WITH_UDP
};

} // namespace inet

#endif // ifndef __INET_PACKETDUMP_H

