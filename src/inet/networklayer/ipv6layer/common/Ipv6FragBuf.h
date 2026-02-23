//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IPV6FRAGBUF_H
#define __INET_IPV6FRAGBUF_H

#include <map>
#include <vector>

#include "inet/common/packet/Packet.h"
#include "inet/common/packet/ReassemblyBuffer.h"
#include "inet/networklayer/contract/ipv6/Ipv6Address.h"

namespace inet {

class Icmpv6;
class Ipv6Header;
class Ipv6FragmentHeader;

/**
 * Reassembly buffer for fragmented Ipv6 datagrams.
 */
class INET_API Ipv6FragBuf
{
  protected:
    //
    // Key for finding the reassembly buffer for a datagram.
    //
    struct Key {
        uint32_t id;
        Ipv6Address src;
        Ipv6Address dest;

        inline bool operator<(const Key& b) const
        {
            return (id != b.id) ? (id < b.id) : (src != b.src) ? (src < b.src) : (dest < b.dest);
        }
    };

    //
    // Reassembly buffer for the datagram
    //
    struct DatagramBuffer {
        ReassemblyBuffer buf; // reassembly buffer
        Packet *packet = nullptr; // the actual datagram
        simtime_t createdAt; // time of the buffer creation (i.e. reception time of first-arriving fragment)
    };

    // we use std::map for fast lookup by datagram Id
    typedef std::map<Key, DatagramBuffer> Buffers;

    // the reassembly buffers
    Buffers bufs;

    // needed for TIME_EXCEEDED errors
    Icmpv6 *icmpModule = nullptr;

  public:
    /**
     * Ctor.
     */
    Ipv6FragBuf();

    /**
     * Dtor.
     */
    ~Ipv6FragBuf();

    /**
     * Initialize fragmentation buffer. ICMP module is needed for sending
     * TIME_EXCEEDED ICMP message in purgeStaleFragments().
     */
    void init(Icmpv6 *icmp);

    /**
     * Takes a fragment and inserts it into the reassembly buffer.
     * If this fragment completes a datagram, the full reassembled
     * datagram is returned, otherwise nullptr.
     */
    Packet *addFragment(Packet *packet, const Ipv6Header *dg, const Ipv6FragmentHeader *fh, simtime_t now);

    /**
     * Throws out all fragments which are incomplete and their
     * last update (last fragment arrival) was before "lastupdate",
     * and sends ICMP TIME EXCEEDED message about them.
     *
     * Timeout should be between 60 seconds and 120 seconds (RFC1122).
     * This method should be called more frequently, maybe every
     * 10..30 seconds or so.
     */
    void purgeStaleFragments(simtime_t lastupdate);
};

} // namespace inet

#endif

