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
#include "ICMP.h"


IPFragBuf::IPFragBuf()
{
    icmpModule = NULL;
}

IPFragBuf::~IPFragBuf()
{
    // FIXME delete "fragments" pointers and datagrams.
}

void IPFragBuf::init(ICMP *icmp)
{
    icmpModule = icmp;
}

IPDatagram *IPFragBuf::addFragment(IPDatagram *datagram, simtime_t now)
{
    Key key;
    key.id = datagram->identification();
    key.src = datagram->srcAddress();
    key.dest = datagram->destAddress();

    Buffers::iterator i = bufs.find(key);

    int bytes = datagram->length()/8 - datagram->headerLength();

    if (i==bufs.end())
    {
        // this is the first fragment of that datagram, create reassembly buffer for it
        ReassemblyBuffer buf;
        buf.main.beg = datagram->fragmentOffset();
        buf.main.end = buf.main.beg + bytes;
        buf.main.islast = !datagram->moreFragments();
        buf.fragments = NULL;
        buf.datagram = datagram;
        buf.lastupdate = now;

        bufs[key] = buf;

        // if datagram is not fragmented, we shouldn't have been called!
        ASSERT(buf.main.beg!=0 || !buf.main.islast);

        return NULL;
    }
    else
    {
        // merge this fragment into reassembly buffer
        ReassemblyBuffer& buf = i->second;
        ushort beg = datagram->fragmentOffset();
        ushort end = beg + bytes;
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
            ret->setLength(8*(ret->headerLength()+buf.main.end));
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
        // most typical case (<95%): new fragment follows last one
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
        // overlapping is not possible;
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
        for (RegionVector::iterator i=frags.begin(); i!=frags.end(); )
        {
            bool deleteit = false;
            Region& frag = *i;
            if (buf.main.end==frag.beg)
            {
                buf.main.end = frag.end;
                if (frag.islast)
                    buf.main.islast = true;
                deleteit = true;
            }
            else if (buf.main.beg==frag.end)
            {
                buf.main.beg = frag.beg;
                deleteit = true;
            }
            else if (buf.main.beg<=frag.beg && buf.main.end>=frag.end)
            {
                // we already have this region (duplicate fragment), delete it
                deleteit = true;
            }

            if (deleteit)
            {
                // deletion is tricky because erase() invalidates iterator
                int pos = i - frags.begin();
                frags.erase(i);
                i = frags.begin() + pos;
                oncemore = true;
            }
            else
            {
                ++i;
            }
        }
    }
    while (oncemore);
}

void IPFragBuf::purgeStaleFragments(simtime_t lastupdate)
{
    // this method shouldn't be called too often because iteration on
    // an std::map is *very* slow...

    ASSERT(icmpModule);

    for (Buffers::iterator i=bufs.begin(); i!=bufs.end(); )
    {
        // if too old, remove it
        ReassemblyBuffer& buf = i->second;
        if (buf.lastupdate < lastupdate)
        {
            // send ICMP error
            ev << "datagram fragment timed out in reassembly buffer, sending ICMP_TIME_EXCEEDED\n";
            icmpModule->sendErrorMessage(buf.datagram, ICMP_TIME_EXCEEDED, 0);

            // delete
            if (buf.fragments)
                delete buf.fragments;
            Buffers::iterator oldi = i++;
            bufs.erase(oldi);
        }
        else
        {
            ++i;
        }
    }
}

