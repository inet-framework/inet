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

#include <stdlib.h>
#include <string.h>

#include "inet/common/INETDefs.h"

#include "inet/networklayer/ipv6/IPv6FragBuf.h"
#include "inet/networklayer/icmpv6/ICMPv6.h"
#include "inet/networklayer/icmpv6/ICMPv6Message_m.h"    // for TIME_EXCEEDED
#include "inet/networklayer/ipv6/IPv6Datagram.h"
#include "inet/networklayer/ipv6/IPv6ExtensionHeaders.h"

namespace inet {

IPv6FragBuf::IPv6FragBuf()
{
    icmpModule = nullptr;
}

IPv6FragBuf::~IPv6FragBuf()
{
    for (auto & elem : bufs) {
        delete elem.second.datagram;
    }
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

    auto i = bufs.find(key);

    DatagramBuffer *buf = nullptr;
    if (i == bufs.end()) {
        // this is the first fragment of that datagram, create reassembly buffer for it
        buf = &bufs[key];
        buf->datagram = nullptr;
        buf->createdAt = now;
    }
    else {
        // use existing buffer
        buf = &(i->second);
    }

    int fragmentLength = datagram->calculateFragmentLength();
    unsigned short offset = fh->getFragmentOffset();
    bool moreFragments = fh->getMoreFragments();

    // RFC 2460 4.5:
    // If the length of a fragment, as derived from the fragment packet's
    // Payload Length field, is not a multiple of 8 octets and the M flag
    // of that fragment is 1, then that fragment must be discarded and an
    // ICMP Parameter Problem, Code 0, message should be sent to the
    // source of the fragment, pointing to the Payload Length field of
    // the fragment packet.
    if (moreFragments && (fragmentLength % 8) != 0) {
        icmpModule->sendErrorMessage(datagram, ICMPv6_PARAMETER_PROBLEM, ERROREOUS_HDR_FIELD);    // TODO set pointer
        return nullptr;
    }

    // RFC 2460 4.5:
    // If the length and offset of a fragment are such that the Payload
    // Length of the packet reassembled from that fragment would exceed
    // 65,535 octets, then that fragment must be discarded and an ICMP
    // Parameter Problem, Code 0, message should be sent to the source of
    // the fragment, pointing to the Fragment Offset field of the
    // fragment packet.
    if (offset + fragmentLength > 65535) {
        icmpModule->sendErrorMessage(datagram, ICMPv6_PARAMETER_PROBLEM, ERROREOUS_HDR_FIELD);    // TODO set pointer
        return nullptr;
    }

    // add fragment to buffer
    bool isComplete = buf->buf.addFragment(offset,
                offset + fragmentLength,
                !moreFragments);

    // Store the first fragment. The first fragment contains the whole
    // encapsulated payload, and extension headers of the
    // original datagram.
    if (offset == 0) {
        delete buf->datagram;
        buf->datagram = datagram;
    }
    else {
        delete datagram;
    }

    // do we have the complete datagram?
    if (isComplete) {
        // datagram complete: deallocate buffer and return complete datagram
        IPv6Datagram *ret = buf->datagram;
        if (!ret)
            throw cRuntimeError("Model error: completed datagram without datagram pointer.");
        ret->removeExtensionHeader(IP_PROT_IPv6EXT_FRAGMENT);
        ret->setByteLength(ret->calculateUnfragmentableHeaderByteLength() + buf->buf.getTotalLength());
        bufs.erase(i);
        return ret;
    }
    else {
        // there are still missing fragments
        return nullptr;
    }
}

/*
 *    If insufficient fragments are received to complete reassembly of a
      packet within 60 seconds of the reception of the first-arriving
      fragment of that packet, reassembly of that packet must be
      abandoned and all the fragments that have been received for that
      packet must be discarded.  If the first fragment (i.e., the one
      with a Fragment Offset of zero) has been received, an ICMP Time
      Exceeded -- Fragment Reassembly Time Exceeded message should be
      sent to the source of that fragment.
 *
 */
void IPv6FragBuf::purgeStaleFragments(simtime_t lastupdate)
{
    // this method shouldn't be called too often because iteration on
    // an std::map is *very* slow...

    ASSERT(icmpModule);

    for (auto i = bufs.begin(); i != bufs.end(); ) {
        // if too old, remove it
        DatagramBuffer& buf = i->second;
        if (buf.createdAt < lastupdate) {
            if (buf.datagram) {
                // send ICMP error
                EV_INFO << "datagram fragment timed out in reassembly buffer, sending ICMP_TIME_EXCEEDED\n";
                icmpModule->sendErrorMessage(buf.datagram, ICMPv6_TIME_EXCEEDED, 0);
            }

            // delete
            auto oldi = i++;
            bufs.erase(oldi);
        }
        else {
            ++i;
        }
    }
}

} // namespace inet

