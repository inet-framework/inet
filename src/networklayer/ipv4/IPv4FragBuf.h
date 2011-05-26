//
// Copyright (C) 2004 Andras Varga
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

#ifndef __INET_IPv4FRAGBUF_H
#define __INET_IPv4FRAGBUF_H


#include <map>

#include "INETDefs.h"

#include "IPv4Address.h"
#include "ReassemblyBuffer.h"


class ICMP;
class IPv4Datagram;


/**
 * Reassembly buffer for fragmented IPv4 datagrams.
 */
class INET_API IPv4FragBuf
{
  protected:
    //
    // Key for finding the reassembly buffer for a datagram.
    //
    struct Key
    {
        ushort id;
        IPv4Address src;
        IPv4Address dest;

        inline bool operator<(const Key& b) const {
            return (id!=b.id) ? (id<b.id) : (src!=b.src) ? (src<b.src) : (dest<b.dest);
        }
    };

    //
    // Reassembly buffer for the datagram
    //
    struct DatagramBuffer
    {
        ReassemblyBuffer buf;  // reassembly buffer
        IPv4Datagram *datagram;  // the actual datagram
        simtime_t lastupdate;  // last time a new fragment arrived
    };

    // we use std::map for fast lookup by datagram Id
    typedef std::map<Key,DatagramBuffer> Buffers;

    // the reassembly buffers
    Buffers bufs;

    // needed for TIME_EXCEEDED errors
    ICMP *icmpModule;

  public:
    /**
     * Ctor.
     */
    IPv4FragBuf();

    /**
     * Dtor.
     */
    ~IPv4FragBuf();

    /**
     * Initialize fragmentation buffer. ICMP module is needed for sending
     * TIME_EXCEEDED ICMP message in purgeStaleFragments().
     */
    void init(ICMP *icmp);

    /**
     * Takes a fragment and inserts it into the reassembly buffer.
     * If this fragment completes a datagram, the full reassembled
     * datagram is returned, otherwise NULL.
     */
    IPv4Datagram *addFragment(IPv4Datagram *datagram, simtime_t now);

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

#endif

