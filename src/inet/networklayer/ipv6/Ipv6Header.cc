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

#include "inet/networklayer/ipv6/Ipv6ExtensionHeaders_m.h"
#include "inet/networklayer/ipv6/Ipv6Header.h"

namespace inet {

std::ostream& operator<<(std::ostream& os, Ipv6ExtensionHeader *eh)
{
    return os << "(" << eh->getClassName() << ") " << eh->str();
}

Ipv6ExtensionHeader *Ipv6Header::findExtensionHeaderByTypeForUpdate(IpProtocolId extensionType, int index)
{
    handleChange();
    return const_cast<Ipv6ExtensionHeader *>(const_cast<Ipv6Header*>(this)->findExtensionHeaderByType(extensionType, index));
}

const Ipv6ExtensionHeader *Ipv6Header::findExtensionHeaderByType(IpProtocolId extensionType, int index) const
{
    for (size_t i=0; i < extensionHeader_arraysize; i++)
        if (extensionHeader[i]->getExtensionType() == extensionType) {
            if (index == 0)
                return extensionHeader[i];
            else
                index--;
        }
    return nullptr;
}

void Ipv6Header::addExtensionHeader(Ipv6ExtensionHeader *eh)
{
    ASSERT((eh->getByteLength() >= B(1)) && (eh->getByteLength().get() % 8 == 0));
    int thisOrder = eh->getOrder();
    size_t i;
    for (i = 0; i < extensionHeader_arraysize; i++) {
        int thatOrder = extensionHeader[i]->getOrder();
        if (thisOrder != -1 && thatOrder > thisOrder)
            break;
        else if (thisOrder == thatOrder) {
            if (thisOrder == 1)   // first IP_PROT_IPv6EXT_DEST has order 1, second IP_PROT_IPv6EXT_DEST has order 6
                thisOrder = 6;
            else if (thisOrder != -1)
                throw cRuntimeError(this, "addExtensionHeader() duplicate extension header: %d",
                        eh->getExtensionType());
        }
    }

    // insert at position atPos, shift up the rest of the array
    insertExtensionHeader(i, eh);
}

/*
 *  Defines the order of extension headers according to RFC 2460 4.1.
 *  Note that Destination Options header may come both after the Hop-by-Hop
 *  extension and before the payload (if Routing presents).
 */
int Ipv6ExtensionHeader::getOrder() const
{
    switch (extensionType) {
        case IP_PROT_IPv6EXT_HOP:
            return 0;

        case IP_PROT_IPv6EXT_DEST:
            return 1;

        case IP_PROT_IPv6EXT_ROUTING:
            return 2;

        case IP_PROT_IPv6EXT_FRAGMENT:
            return 3;

        case IP_PROT_IPv6EXT_AUTH:
            return 4;

        case IP_PROT_IPv6EXT_ESP:
            return 5;

        // second IP_PROT_IPv6EXT_DEST has order 6

        case IP_PROT_IPv6EXT_MOB:
            return 7;

        default:
            return -1;
    }
}

B Ipv6Header::calculateHeaderByteLength() const
{
    B len = B(40);
    for (size_t i = 0; i < extensionHeader_arraysize; i++)
        len += extensionHeader[i]->getByteLength();
    return len;
}

/**
 * Note: it is assumed that headers are ordered as described in RFC 2460 4.1
 */
B Ipv6Header::calculateUnfragmentableHeaderByteLength() const
{
    size_t firstFragmentableExtensionIndex = 0;
    for (size_t i = extensionHeader_arraysize; i > 0; i--) {
        int type = extensionHeader[i-1]->getExtensionType();
        if (type == IP_PROT_IPv6EXT_ROUTING || type == IP_PROT_IPv6EXT_HOP) {
            firstFragmentableExtensionIndex = i;
            break;
        }
    }

    B len = B(40);
    for (size_t i = 0; i < firstFragmentableExtensionIndex; i++)
        len += extensionHeader[i]->getByteLength();
    return len;
}

/**
 * Note: it is assumed that headers are ordered as described in RFC 2460 4.1
 */
B Ipv6Header::calculateFragmentLength() const
{
    B len = getChunkLength() - IPv6_HEADER_BYTES;
    size_t i;
    for (i = 0; i < extensionHeader_arraysize; i++) {
        len -= extensionHeader[i]->getByteLength();
        if (extensionHeader[i]->getExtensionType() == IP_PROT_IPv6EXT_FRAGMENT)
            break;
    }
    ASSERT2(i < extensionHeader_arraysize, "IPv6Datagram::calculateFragmentLength() called on non-fragment datagram");
    return len;
}

Ipv6ExtensionHeader *Ipv6Header::removeFirstExtensionHeader()
{
    handleChange();
    if (extensionHeader_arraysize == 0)
        return nullptr;
    Ipv6ExtensionHeader *eh = dropExtensionHeader(0);
    eraseExtensionHeader(0);
    return eh;
}

Ipv6ExtensionHeader *Ipv6Header::removeExtensionHeader(IpProtocolId extensionType)
{
    handleChange();
    if (extensionHeader_arraysize == 0)
        return nullptr;

    for (size_t i = 0; i < extensionHeader_arraysize; i++) {
        if (extensionHeader[i]->getExtensionType() == extensionType) {
            Ipv6ExtensionHeader *eh = dropExtensionHeader(i);
            eraseExtensionHeader(i);
            return eh;
        }
    }
    return nullptr;
}

short Ipv6Header::getDscp() const
{
    return (trafficClass & 0xfc) >> 2;
}

void Ipv6Header::setDscp(short dscp)
{
    setTrafficClass(((dscp & 0x3f) << 2) | (trafficClass & 0x03));
}

short Ipv6Header::getEcn() const
{
    return trafficClass & 0x03;
}

void Ipv6Header::setEcn(short ecn)
{
    setTrafficClass((trafficClass & 0xfc) | (ecn & 0x03));
}

std::ostream& operator<<(std::ostream& out, const Ipv6ExtensionHeader& h)
{
    out << "{type:" << h.getExtensionType() << ",length:" << h.getByteLength() << "}";
    return out;
}

} // namespace inet

