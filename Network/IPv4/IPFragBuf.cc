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


IPFragBuf::IPFragBuf()
{
}

IPFragBuf::~IPFragBuf()
{
    // FIXME delete "fragments" pointers and datagrams.
}

IPDatagram *IPFragBuf::addFragment(IPDatagram *datagram, simtime_t now)
{
    ushort id = datagram->identification();
    Buffers::iterator i = bufs.find(id);
    if (i==bufs.end())
    {
        // this is the first fragment of that datagram, create reassembly buffer for it
        ReassemblyBuffer buf;
        buf.id = id;
        buf.main.beg = datagram->fragmentOffset();
        buf.main.end = buf.main.beg + datagram->payloadLength(); //FIXME
        buf.main.islast = !datagram->moreFragments();
        buf.fragments = NULL;
        buf.datagram = datagram;
        buf.lastupdate = now;

        bufs[id] = buf;

        // if datagram is not fragmented, we shouldn't have been called!
        ASSERT(buf.main.beg!=0 || !buf.main.islast);

        return NULL;
    }
    else
    {
        // merge this fragment into reassembly buffer
        ReassemblyBuffer& buf = *i;
        ushort beg = datagram->fragmentOffset();
        ushort end = buf.main.beg + datagram->payloadLength(); //FIXME
        merge(buf, beg, end, !datagram->moreFragments());

        // store datagram. Only one fragment carries the actual modelled
        // content (encapsulatedMsg()), other (empty) ones are only
        // preserved so that we can send them in ICMP if reassembly times out.
        if (datagram->encapsulatedMsg())
        {
            delete buf.datagram;
            buf.datagram = datagram;
        }
        else
        {
            delete datagram;
        }

        // do we have the complete datagram?
        if (buf.main.beg==0 && buf.main.islast)
        {
            // datagram complete: deallocate buffer and return complete datagram
            IPDatagram *ret = buf.datagram;
            if (buf.fragments)
                delete buf.fragments;
            bufs.erase(i);
            return ret;
        }
        else
        {
            // there are still missing fragments
            buf.lastupdate = now;
            return NULL;
        }
    }
}

void IPFragBuf::merge(ReassemblyBuffer& buf, ushort beg, ushort end, bool islast)
{
    if (buf.main.end==beg)
    {
        // most typycal case (<95%): new fragment follows last one
        buf.main.end = end;
        if (islast)
            buf.main.islast = true;
        if (buf.fragments)
            mergeFragments(buf);
    }
    else if (buf.main.beg==end)
    {
        // new fragment precedes what we already have
        buf.main.beg = beg;
        if (buf.fragments)
            mergeFragments(buf);
    }
    else if (buf.main.end<beg || buf.main.beg>end)
    {
        // disjoint fragment, store it until another fragment fills in the gap
        if (!buf.fragments)
            buf.fragments = new RegionVector();
        Region r;
        r.beg = beg;
        r.end = end;
        r.islast = islast;
        buf.fragments->push_back(r);
    }
    else
    {
        // fragment's range already contained in buffer (probably duplicate fragment)
    }
}

void IPFragBuf::mergeFragments(ReassemblyBuffer& buf)
{
    RegionVector& frags = *(buf.fragments);

    bool oncemore;
    do
    {
        oncemore = false;
        for (RegionVector::iterator i=frags.begin(); i!=frags.end(); ++i)
        {
            Region& frag = *i;
            if (buf.main.end==frag.beg)
            {
                buf.main.end = frag.end;
                if (frag.islast)
                    buf.main.islast = true;
                // delete this frag!!!!!!!!!!
            }
            else if (buf.main.beg==end)
            {
                buf.main.beg = frag.beg;
                // delete this frag
            }
            else if (buf.main.beg<=frag.beg && buf.main.end>=frag.end)
            {
                // we already have this region (duplicate fragment), delete it
                //delete!!!!!!!!!!
            }
        }
    }
    while (oncemore);
}

void IPFragBuf::purgeStaleFragments(simtime_t lastupdate)
{
    for (Buffers::iterator i=bufs.begin(); i!=bufs.end(); ++i)
    {
        if (i->lastupdate < lastupdate)
        {
            // send ICMP!!!

            //delete!!!!
        }
    }
}

