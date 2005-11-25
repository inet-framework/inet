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
}

void IPFragBuf::init(ICMP *icmp)
{
    icmpModule = icmp;
}

IPDatagram *IPFragBuf::addFragment(IPDatagram *datagram, simtime_t now)
{
    // find datagram buffer
    Key key;
    key.id = datagram->identification();
    key.src = datagram->srcAddress();
    key.dest = datagram->destAddress();

    Buffers::iterator i = bufs.find(key);

    DatagramBuffer *buf = NULL;
    if (i==bufs.end())
    {
        // this is the first fragment of that datagram, create reassembly buffer for it
        buf = &bufs[key];
        buf->datagram = NULL;
    }
    else
    {
        // use existing buffer
        buf = &(i->second);
    }

    // add fragment into reassembly buffer
    int bytes = datagram->byteLength() - datagram->headerLength();
    bool isComplete = buf->buf.addFragment(datagram->fragmentOffset(),
                                           datagram->fragmentOffset() + bytes,
                                           !datagram->moreFragments());

    // store datagram. Only one fragment carries the actual modelled
    // content (encapsulatedMsg()), other (empty) ones are only
    // preserved so that we can send them in ICMP if reassembly times out.
    if (datagram->encapsulatedMsg())
    {
        delete buf->datagram;
        buf->datagram = datagram;
    }
    else
    {
        delete datagram;
    }

    // do we have the complete datagram?
    if (isComplete)
    {
        // datagram complete: deallocate buffer and return complete datagram
        IPDatagram *ret = buf->datagram;
        ret->setByteLength(ret->headerLength()+buf->buf.totalLength());
        bufs.erase(i);
        return ret;
    }
    else
    {
        // there are still missing fragments
        buf->lastupdate = now;
        return NULL;
    }
}

void IPFragBuf::purgeStaleFragments(simtime_t lastupdate)
{
    // this method shouldn't be called too often because iteration on
    // an std::map is *very* slow...

    ASSERT(icmpModule);

    for (Buffers::iterator i=bufs.begin(); i!=bufs.end(); )
    {
        // if too old, remove it
        DatagramBuffer& buf = i->second;
        if (buf.lastupdate < lastupdate)
        {
            // send ICMP error
            EV << "datagram fragment timed out in reassembly buffer, sending ICMP_TIME_EXCEEDED\n";
            icmpModule->sendErrorMessage(buf.datagram, ICMP_TIME_EXCEEDED, 0);

            // delete
            Buffers::iterator oldi = i++;
            bufs.erase(oldi);
        }
        else
        {
            ++i;
        }
    }
}

