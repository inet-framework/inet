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

#include "inet/networklayer/ipv4/IPv4FragBuf.h"

//#include "inet/common/RawPacket.h"
//#include "inet/common/serializer/SerializerBase.h"
#include "inet/networklayer/ipv4/ICMP.h"
#include "inet/networklayer/ipv4/IPv4Header.h"

namespace inet {

//TODO need solution for fragments with encapsulated RawPacket contains bytes of fragment or bytes of total packet

IPv4FragBuf::IPv4FragBuf()
{
}

IPv4FragBuf::~IPv4FragBuf()
{
    for (auto i = bufs.begin(); i != bufs.end(); ) {
        delete i->second.packet;
    }
    bufs.clear();
}

Packet *IPv4FragBuf::addFragment(Packet *packet, simtime_t now)
{
    const auto& datagram = packet->peekHeader<IPv4Header>();
    // find datagram buffer
    Key key;
    key.id = datagram->getIdentification();
    key.src = datagram->getSrcAddress();
    key.dest = datagram->getDestAddress();

    auto i = bufs.find(key);

    DatagramBuffer *buf = nullptr;

    if (i == bufs.end()) {
        // this is the first fragment of that datagram, create reassembly buffer for it
        buf = &bufs[key];
        buf->packet = nullptr;
    }
    else {
        // use existing buffer
        buf = &(i->second);
    }

    // add fragment into reassembly buffer
    int bytes = datagram->getTotalLengthField() - datagram->getHeaderLength();
    buf->buf.replace(datagram->getFragmentOffset(), packet->peekDataAt(datagram->getHeaderLength(), bytes));
    if (datagram->getFragmentOffset() == 0 || buf->packet == nullptr) {
        delete buf->packet;
        buf->packet = packet;
    }
    else {
        delete packet;
    }
    if (!datagram->getMoreFragments()) {
        buf->buf.setExpectedLength(datagram->getFragmentOffset() + bytes);
    }

    // do we have the complete datagram?
    if (buf->buf.isComplete()) {
        // datagram complete: deallocate buffer and return complete datagram
        std::string pkName(buf->packet->getName());
        std::size_t found = pkName.find("-frag-");
        if (found != std::string::npos)
            pkName.resize(found);
        Packet *pk = new Packet(pkName.c_str());
        pk->transferTagsFrom(buf->packet);
        auto hdr = std::shared_ptr<IPv4Header>(buf->packet->peekHeader<IPv4Header>()->dup());
        const auto& payload = buf->buf.getData();
        hdr->setTotalLengthField(hdr->getHeaderLength() + payload->getChunkLength());
        hdr->setFragmentOffset(0);
        hdr->setMoreFragments(false);
        pk->pushHeader(hdr);
        pk->pushTrailer(payload);
        delete buf->packet;
        bufs.erase(i);
        return pk;
    }
    else {
        // there are still missing fragments
        buf->lastupdate = now;
        return nullptr;
    }
}

void IPv4FragBuf::purgeStaleFragments(ICMP *icmpModule, simtime_t lastupdate)
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

