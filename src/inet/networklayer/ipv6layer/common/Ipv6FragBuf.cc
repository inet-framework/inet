//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/ipv6/Ipv6FragBuf.h"

#include <stdlib.h>
#include <string.h>

#include "inet/networklayer/icmpv6/Icmpv6.h"
#include "inet/networklayer/icmpv6/Icmpv6Header_m.h"
#include "inet/networklayer/ipv6/Ipv6ExtensionHeaders_m.h"
#include "inet/networklayer/ipv6/Ipv6Header.h"

namespace inet {

Ipv6FragBuf::Ipv6FragBuf()
{
    icmpModule = nullptr;
}

Ipv6FragBuf::~Ipv6FragBuf()
{
    for (auto& elem : bufs) {
        delete elem.second.packet;
    }
}

void Ipv6FragBuf::init(Icmpv6 *icmp)
{
    icmpModule = icmp;
}

Packet *Ipv6FragBuf::addFragment(Packet *pk, const Ipv6Header *ipv6Header, const Ipv6FragmentHeader *fh, simtime_t now)
{
    // find datagram buffer
    Key key;
    key.id = fh->getIdentification();
    key.src = ipv6Header->getSrcAddress();
    key.dest = ipv6Header->getDestAddress();

    auto i = bufs.find(key);

    DatagramBuffer *buf = nullptr;
    if (i == bufs.end()) {
        // this is the first fragment of that datagram, create reassembly buffer for it
        buf = &bufs[key];
        buf->packet = nullptr;
        buf->createdAt = now;
    }
    else {
        // use existing buffer
        buf = &(i->second);
    }

    int fragmentLength = pk->getByteLength() - B(ipv6Header->getChunkLength()).get(); // datagram->calculateFragmentLength();
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
        icmpModule->sendErrorMessage(pk, ICMPv6_PARAMETER_PROBLEM, ERROREOUS_HDR_FIELD); // TODO set pointer
        delete pk;
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
        icmpModule->sendErrorMessage(pk, ICMPv6_PARAMETER_PROBLEM, ERROREOUS_HDR_FIELD); // TODO set pointer
        delete pk;
        return nullptr;
    }

    // add fragment to buffer
    buf->buf.replace(B(offset), pk->peekDataAt(ipv6Header->getChunkLength(), B(fragmentLength)));

    if (!moreFragments) {
        buf->buf.setExpectedLength(B(offset + fragmentLength));
    }

    // Store the first fragment. The first fragment contains the whole
    // encapsulated payload, and extension headers of the
    // original datagram.
    if (offset == 0) {
        delete buf->packet;
        buf->packet = pk;
    }
    else {
        delete pk;
    }

    // do we have the complete datagram?
    if (buf->buf.isComplete()) {
        // datagram complete: deallocate buffer and return complete datagram
        std::string pkName(buf->packet->getName());
        std::size_t found = pkName.find("-frag-");
        if (found != std::string::npos)
            pkName.resize(found);
        Packet *pk = new Packet(pkName.c_str());
        pk->copyTags(*buf->packet);
        auto hdr = Ptr<Ipv6Header>(buf->packet->peekAtFront<Ipv6Header>()->dup());
        const auto& payload = buf->buf.getReassembledData();
        hdr->removeExtensionHeader(IP_PROT_IPv6EXT_FRAGMENT);
        hdr->setChunkLength(B(hdr->calculateUnfragmentableHeaderByteLength()));
        pk->insertAtFront(hdr);
        pk->insertAtBack(payload);
        delete buf->packet;
        bufs.erase(i);
        return pk;
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
void Ipv6FragBuf::purgeStaleFragments(simtime_t lastupdate)
{
    // this method shouldn't be called too often because iteration on
    // an std::map is *very* slow...

    ASSERT(icmpModule);

    for (auto i = bufs.begin(); i != bufs.end();) {
        // if too old, remove it
        DatagramBuffer& buf = i->second;
        if (buf.createdAt < lastupdate) {
            if (buf.packet) {
                // send ICMP error
                EV_INFO << "datagram fragment timed out in reassembly buffer, sending ICMP_TIME_EXCEEDED\n";
                icmpModule->sendErrorMessage(buf.packet, ICMPv6_TIME_EXCEEDED, 0);
                delete buf.packet;
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

