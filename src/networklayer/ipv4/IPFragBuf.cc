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
    key.id = datagram->getIdentification();
    key.src = datagram->getSrcAddress();
    key.dest = datagram->getDestAddress();

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
    int bytes = datagram->getByteLength() - datagram->getHeaderLength();
    bool isComplete = buf->buf.addFragment(datagram->getFragmentOffset(),
                                           datagram->getFragmentOffset() + bytes,
                                           !datagram->getMoreFragments());

    // store datagram. Only one fragment carries the actual modelled
    // content (getEncapsulatedPacket()), other (empty) ones are only
    // preserved so that we can send them in ICMP if reassembly times out.
    if (datagram->getEncapsulatedPacket())
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
        ret->setByteLength(ret->getHeaderLength()+buf->buf.getTotalLength());
        ret->setFragmentOffset(0);
        ret->setMoreFragments(false);
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
            // send ICMP error.
            // Note: receiver MUST NOT call decapsulate() on the datagram fragment,
            // because its length (being a fragment) is smaller than the encapsulated
            // packet, resulting in "length became negative" error. Use getEncapsulatedPacket().
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

