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


#include <omnetpp.h>
#include <stdlib.h>
#include <string.h>
#include "IPv6FragBuf.h"
#include "ICMPv6.h"
#include "ICMPv6Message_m.h"  // for TIME_EXCEEDED
#include "IPv6Datagram.h"
#include "IPv6ExtensionHeaders_m.h"


IPv6FragBuf::IPv6FragBuf()
{
    icmpModule = NULL;
}

IPv6FragBuf::~IPv6FragBuf()
{
}

void IPv6FragBuf::init(ICMPv6 *icmp)
{
    icmpModule = icmp;
}

IPv6Datagram *IPv6FragBuf::addFragment(IPv6Datagram *datagram, IPv6FragmentHeader *fh, simtime_t now)
{
    // find datagram buffer
    Key key;
    key.id = fh->getIdentification();
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
    // FIXME next lines aren't correct: check 4.5 of RFC 2460 regarding Unfragmentable part, Fragmentable part, etc
    int bytes = datagram->getByteLength() - datagram->calculateHeaderByteLength();
    bool isComplete = buf->buf.addFragment(fh->getFragmentOffset(),
                                           fh->getFragmentOffset() + bytes,
                                           !fh->getMoreFragments());

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
        IPv6Datagram *ret = buf->datagram;
        ret->setByteLength(ret->calculateHeaderByteLength()+buf->buf.getTotalLength()); // FIXME cf with 4.5 of RFC 2460
        //TODO: remove extension header IPv6FragmentHeader; maybe not here but when datagram gets inserted into the reassembly buffer --Andras
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

void IPv6FragBuf::purgeStaleFragments(simtime_t lastupdate)
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
            icmpModule->sendErrorMessage(buf.datagram, ICMPv6_TIME_EXCEEDED, 0);

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

