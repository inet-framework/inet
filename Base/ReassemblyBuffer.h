//
// Copyright (C) 2004-2005 Andras Varga
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

#ifndef __REASSEMBLYBUFFER_H__
#define __REASSEMBLYBUFFER_H__

#include <map>
#include <vector>
#include "INETDefs.h"


/**
 * Generic reassembly buffer for a fragmented datagram (or a fragmented anything).
 *
 * Currently used in IPFragBuf and IPv6FragBuf.
 */
class INET_API ReassemblyBuffer
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

    //
    // Thinking of IP/IPv6 fragmentation, 99% of the time fragments
    // will arrive in order and none gets lost, so we have to
    // handle this case very efficiently. For this purpose
    // we'll store the offset of the first and last+1 byte we have
    // (main.beg, main.end variables), and keep extending this range
    // as new fragments arrive. If we receive non-connecting fragments,
    // put them aside into buf until new fragments come and fill the gap.
    //
    Region main;   // offset range we already have
    RegionVector *fragments;  // only used if we receive disjoint fragments

  protected:
    void merge(ushort beg, ushort end, bool islast);
    void mergeFragments();

  public:
    /**
     * Ctor.
     */
    ReassemblyBuffer();

    /**
     * Dtor.
     */
    ~ReassemblyBuffer();

    /**
     * Add a fragment, and returns true if reassembly has completed
     * (i.e. we have everything from offset 0 to the last fragment).
     */
    bool addFragment(ushort beg, ushort end, bool islast);

    /**
     * Returns the total (assembled) length of the datagram.
     * Can only be called after addFragment() returned true.
     */
    ushort totalLength() const {return main.end;}
};

#endif

