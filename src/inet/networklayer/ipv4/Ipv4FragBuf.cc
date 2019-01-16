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

#include <stdlib.h>
#include <string.h>

#include "inet/networklayer/ipv4/Icmp.h"
#include "inet/networklayer/ipv4/Ipv4FragBuf.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"

namespace inet {

Ipv4FragBuf::Ipv4FragBuf()
{
}

Ipv4FragBuf::~Ipv4FragBuf()
{
    flush();
}

void Ipv4FragBuf::flush()
{
    for (auto i = bufs.begin(); i != bufs.end(); ++i)
        delete i->second.packet;
    bufs.clear();
}

Packet *Ipv4FragBuf::addFragment(Packet *packet, simtime_t now)
{
    const auto& ipv4Header = packet->peekAtFront<Ipv4Header>();
    // find datagram buffer
    Key key;
    key.id = ipv4Header->getIdentification();
    key.src = ipv4Header->getSrcAddress();
    key.dest = ipv4Header->getDestAddress();

    auto i = bufs.find(key);

    DatagramBuffer *curBuf = nullptr;

    if (i == bufs.end()) {
        // this is the first fragment of that datagram, create reassembly buffer for it
        curBuf = &bufs[key];
        i = bufs.find(key);
    }
    else {
        // use existing buffer
        curBuf = &(i->second);
    }

    // add fragment into reassembly buffer
    ASSERT(ipv4Header->getTotalLengthField() > ipv4Header->getHeaderLength());
    B bytes = ipv4Header->getTotalLengthField() - ipv4Header->getHeaderLength();
    curBuf->buf.replace(B(ipv4Header->getFragmentOffset()), packet->peekDataAt(B(ipv4Header->getHeaderLength()), bytes));
    if (!ipv4Header->getMoreFragments()) {
        curBuf->buf.setExpectedLength(B(ipv4Header->getFragmentOffset()) + bytes);
    }
    if (ipv4Header->getFragmentOffset() == 0 || curBuf->packet == nullptr) {
        delete curBuf->packet;
        curBuf->packet = packet;
    }
    else {
        delete packet;
    }

    // do we have the complete datagram?
    if (curBuf->buf.isComplete()) {
        // datagram complete: deallocate buffer and return complete datagram
        std::string pkName(curBuf->packet->getName());
        std::size_t found = pkName.find("-frag-");
        if (found != std::string::npos)
            pkName.resize(found);
        auto hdr = Ptr<Ipv4Header>(curBuf->packet->peekAtFront<Ipv4Header>()->dup());
        Packet *pk = curBuf->packet;
        pk->setName(pkName.c_str());
        pk->removeAll();
        const auto& payload = curBuf->buf.getReassembledData();
        hdr->setTotalLengthField(hdr->getHeaderLength() + payload->getChunkLength());
        hdr->setFragmentOffset(0);
        hdr->setMoreFragments(false);
        pk->insertAtFront(hdr);
        pk->insertAtBack(payload);
        bufs.erase(i);
        return pk;
    }
    else {
        // there are still missing fragments
        curBuf->lastupdate = now;
        return nullptr;
    }
}

void Ipv4FragBuf::purgeStaleFragments(Icmp *icmpModule, simtime_t lastupdate)
{
    // this method shouldn't be called too often because iteration on
    // an std::map is *very* slow...

    ASSERT(icmpModule);

    for (auto i = bufs.begin(); i != bufs.end(); ) {
        // if too old, remove it
        DatagramBuffer& buf = i->second;
        if (buf.lastupdate < lastupdate) {
            // send ICMP error.
            // Note: receiver MUST NOT call decapsulate() on the datagram fragment,
            // because its length (being a fragment) is smaller than the encapsulated
            // packet, resulting in "length became negative" error. Use getEncapsulatedPacket().
            EV_WARN << "datagram fragment timed out in reassembly buffer, sending ICMP_TIME_EXCEEDED\n";
            if (buf.packet != nullptr)
                icmpModule->sendErrorMessage(buf.packet, -1    /*TODO*/, ICMP_TIME_EXCEEDED, 0);

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

