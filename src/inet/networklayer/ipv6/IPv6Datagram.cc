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

#include "inet/networklayer/ipv6/IPv6Datagram.h"
#include "inet/networklayer/ipv6/IPv6ExtensionHeaders.h"

namespace inet {

Register_Class(IPv6Datagram);

IPv6ExtensionHeader *IPv6Datagram::findExtensionHeaderByType(IPProtocolId extensionType, int index) const
{
    for (size_t i = 0; i < extensionHeader_arraysize; i++) {
        if (extensionHeader[i]->getExtensionType() == extensionType) {
            if (index == 0)
                return extensionHeader[i];
            else
                index--;
        }
    }
    return nullptr;
}

void IPv6Datagram::addExtensionHeader(IPv6ExtensionHeader *eh)
{
    ASSERT((eh->getByteLength() >= 1) && (eh->getByteLength() % 8 == 0));
    int thisOrder = getExtensionHeaderOrder(eh);
    size_t i;
    for (i = 0; i < extensionHeader_arraysize; i++) {
        int thatOrder = getExtensionHeaderOrder(extensionHeader[i]);
        if (thisOrder != -1 && thatOrder > thisOrder)
            break;
        else if (thisOrder == thatOrder) {
            if (thisOrder == 1)
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
int IPv6Datagram::getExtensionHeaderOrder(IPv6ExtensionHeader *eh)
{
    switch (eh->getExtensionType()) {
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

int IPv6Datagram::calculateHeaderByteLength() const
{
    int len = 40;
    for (size_t i = 0; i < extensionHeader_arraysize; i++)
        len += extensionHeader[i]->getByteLength();
    return len;
}

/**
 * Note: it is assumed that headers are ordered as described in RFC 2460 4.1
 */
int IPv6Datagram::calculateUnfragmentableHeaderByteLength() const
{
    int lastUnfragmentableExtensionIndex = -1;
    for (int i = ((int)extensionHeader_arraysize) - 1; i >= 0; i--) {
        int type = extensionHeader[i]->getExtensionType();
        if (type == IP_PROT_IPv6EXT_ROUTING || type == IP_PROT_IPv6EXT_HOP) {
            lastUnfragmentableExtensionIndex = i;
            break;
        }
    }

    int len = 40;
    for (int i = 0; i <= lastUnfragmentableExtensionIndex; i++)
        len += extensionHeader[i]->getByteLength();
    return len;
}

/**
 * Note: it is assumed that headers are ordered as described in RFC 2460 4.1
 */
int IPv6Datagram::calculateFragmentLength() const
{
    int len = getByteLength() - IPv6_HEADER_BYTES;
    size_t i;
    for (i = 0; i < extensionHeader_arraysize; i++) {
        len -= extensionHeader[i]->getByteLength();
        if (extensionHeader[i]->getExtensionType() == IP_PROT_IPv6EXT_FRAGMENT)
            break;
    }
    ASSERT2(i < extensionHeader_arraysize, "IPv6Datagram::calculateFragmentLength() called on non-fragment datagram");
    return len;
}

IPv6ExtensionHeader *IPv6Datagram::removeFirstExtensionHeader()
{
    if (extensionHeader_arraysize == 0)
        return nullptr;
    IPv6ExtensionHeader *hdr = dropExtensionHeader(0);
    eraseExtensionHeader(0);
    return hdr;
}

IPv6ExtensionHeader *IPv6Datagram::removeExtensionHeader(IPProtocolId extensionType)
{
    for (size_t i = 0; i < extensionHeader_arraysize; i++) {
        if (extensionHeader[i]->getExtensionType() == extensionType) {
            IPv6ExtensionHeader *hdr = dropExtensionHeader(i);
            eraseExtensionHeader(i);
            return hdr;
        }
    }
    return nullptr;
}

IPv6Datagram::~IPv6Datagram()
{
}

std::ostream& operator<<(std::ostream& out, const IPv6ExtensionHeader& h)
{
    out << "{type:" << h.getExtensionType() << ",length:" << h.getByteLength() << "}";
    return out;
}

} // namespace inet

