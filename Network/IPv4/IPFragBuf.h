//
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#ifndef __IPFRAGBUF_H__
#define __IPFRAGBUF_H__

#include <map>
#include <vector>
#include "INETDefs.h"
#include "IPDatagram.h"


class ICMP;


/**
 * Reassembly buffer for fragmented IP datagrams.
 */
class IPFragBuf
{
  protected:
    // stores an offset range
    struct Region
    {
        ushort beg;   // first offset stored
        ushort end;   // last+1 offset stored
        bool islast;  // if this region represents the last bytes of the datagram
    };

    typedef std::vector<Region> RegionVector;

    /**
     * Key for finding the reassembly buffer for a datagram.
     */
    struct Key
    {
        ushort id;
        IPAddress src;
        IPAddress dest;

        inline bool operator<(const Key& b) const {
            return (id!=b.id) ? (id<b.id) : (src!=b.src) ? (src<b.src) : (dest<b.dest);
        }
    };

    /**
     * Represents the buffer for assembling one IP datagram from fragments.
     * 99% of time, fragments will arrive in order and none gets lost,
     * so we have to handle this case very efficiently. For this purpose
     * we'll store the offset of the first and last+1 byte we have
     * (main.beg, main.end variables), and keep extending this range
     * as new fragments arrive. If we receive non-connecting fragments,
     * put them aside into buf until new fragments come and fill the gap.
     */
    struct ReassemblyBuffer
    {
        Region main;   // offset range we already have
        RegionVector *fragments;  // only used if we receive disjoint fragments
        IPDatagram *datagram;  // the actual datagram
        simtime_t lastupdate;  // last time a new fragment arrived
    };

    // we use std::map for fast lookup by datagram Id
    typedef std::map<Key,ReassemblyBuffer> Buffers;

    // the reassembly buffers
    Buffers bufs;

    // needed for TIME_EXCEEDED errors
    ICMP *icmpModule;

  protected:
    void merge(ReassemblyBuffer& buf, ushort beg, ushort end, bool islast);
    void mergeFragments(ReassemblyBuffer& buf);

  public:
    /**
     * Ctor.
     */
    IPFragBuf();

    /**
     * Dtor.
     */
    ~IPFragBuf();

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
    IPDatagram *addFragment(IPDatagram *datagram, simtime_t now);

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

