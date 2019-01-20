//
// Copyright (C) 2005 Andras Varga
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

#ifndef __INET_IPV6FRAGBUF_H
#define __INET_IPV6FRAGBUF_H

#include <map>
#include <vector>

#include "inet/common/INETDefs.h"
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
    struct Key
    {
        uint32 id;
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
    struct DatagramBuffer
    {
        ReassemblyBuffer buf;    // reassembly buffer
        Packet *packet = nullptr;    // the actual datagram
        simtime_t createdAt;    // time of the buffer creation (i.e. reception time of first-arriving fragment)
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

#endif // ifndef __INET_IPV6FRAGBUF_H

