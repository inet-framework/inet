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


#include <omnetpp.h>
#include <stdlib.h>
#include <string.h>

#include "IPFragBuf.h"


    // stores an offset range
    struct Region
    {
        ushort begin;  // first offset stored
        ushort end;    // last+1 offset stored
    };

    typedef std::vector<Region> RegionVector;

    /**
     * Represents the buffer for assembling one IP datagram from fragments.
     * 99% of time, fragments will arrive in order and none gets lost,
     * so we have to handle this case very efficiently. For this purpose
     * we'll store the offset of the first and last+1 byte we have
     * (main.begin, main.end variables), and keep extending this range
     * as new fragments arrive. If we receive non-connecting fragments,
     * put them aside into buf until new fragments come and fill the gap.
     */
    struct ReassemblyBuffer
    {
        ushort id;     // "Identification" field from IP header
        ushort length; // total length of datagram (without header)
        Region main;   // offset range we already have
        RegionVector *fragments;  // only used if we receive disjoint fragments
        IPDatagram *datagram;  // the actual datagram
        simtime_t lastupdate;  // last time a new fragment arrived
    };

    // we use std::map for fast lookup by datagram Id
    typedef std::map<ushort,ReassemblyBuffer> FragmentationMap;


IPFragBuf::IPFragBuf()
{
}

IPFragBuf::~IPFragBuf()
{
    // FIXME delete "fragments" pointers and datagrams.
}

IPDatagram *IPFragBuf::addFragment(IPDatagram *datagram, simtime_t now)
{
}

void IPFragBuf::purgeStaleFragments(simtime_t lastTouched)
{
}

