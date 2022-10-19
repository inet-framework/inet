//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IPV4FRAGBUF_H
#define __INET_IPV4FRAGBUF_H

#include <map>

#include "inet/common/packet/Packet.h"
#include "inet/common/packet/ReassemblyBuffer.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"

namespace inet {

class Icmp;
class Ipv4Header;

/**
 * Reassembly buffer for fragmented Ipv4 datagrams.
 */
class INET_API Ipv4FragBuf
{
  protected:
    //
    // Key for finding the reassembly buffer for a datagram.
    //
    struct Key {
        ushort id = static_cast<ushort>(-1);
        Ipv4Address src;
        Ipv4Address dest;

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
        Packet *packet = nullptr; // the packet
        simtime_t lastupdate; // last time a new fragment arrived
    };

    // we use std::map for fast lookup by datagram Id
    typedef std::map<Key, DatagramBuffer> Buffers;

    // the reassembly buffers
    Buffers bufs;

  public:
    /**
     * Ctor.
     */
    Ipv4FragBuf();

    /**
     * Dtor.
     */
    ~Ipv4FragBuf();

    /**
     * Takes a fragment and inserts it into the reassembly buffer.
     * If this fragment completes a datagram, the full reassembled
     * datagram is returned, otherwise nullptr.
     */
    Packet *addFragment(Packet *packet, simtime_t now);

    /**
     * Throws out all fragments which are incomplete and their
     * last update (last fragment arrival) was before "lastupdate",
     * and sends ICMP TIME EXCEEDED message about them.
     *
     * Timeout should be between 60 seconds and 120 seconds (RFC1122).
     * This method should be called more frequently, maybe every
     * 10..30 seconds or so.
     */
    void purgeStaleFragments(Icmp *icmpModule, simtime_t lastupdate);

    /**
     * Clear all state.
     */
    void flush();
};

} // namespace inet

#endif

