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

#include "inet/networklayer/ipv6/Ipv6Header.h"
#include "inet/networklayer/ipv6/IPv6ExtensionHeaders.h"

namespace inet {

Register_Class(Ipv6Header);

std::ostream& operator<<(std::ostream& os, IPv6ExtensionHeaderPtr eh)
{
    return os << "(" << eh->getClassName() << ") " << eh->str();
}

Ipv6Header& Ipv6Header::operator=(const Ipv6Header& other)
{
    if (this == &other)
        return *this;
    clean();
    Ipv6Header_Base::operator=(other);
    copy(other);
    return *this;
}

void Ipv6Header::copy(const Ipv6Header& other)
{
    for (const auto & elem : other.extensionHeaders)
        addExtensionHeader((elem)->dup());
}

void Ipv6Header::setExtensionHeaderArraySize(unsigned int size)
{
    throw cRuntimeError(this, "setExtensionHeaderArraySize() not supported, use addExtensionHeader()");
}

unsigned int Ipv6Header::getExtensionHeaderArraySize() const
{
    return extensionHeaders.size();
}

IPv6ExtensionHeaderPtr& Ipv6Header::getMutableExtensionHeader(unsigned int k)
{
    static IPv6ExtensionHeaderPtr null;
    handleChange();
    if (k >= extensionHeaders.size())
        return null = nullptr;
    return extensionHeaders[k];
}

const IPv6ExtensionHeaderPtr& Ipv6Header::getExtensionHeader(unsigned int k) const
{
    static IPv6ExtensionHeaderPtr null;
    if (k >= extensionHeaders.size())
        return null = nullptr;
    return extensionHeaders[k];
}

Ipv6ExtensionHeader *Ipv6Header::findMutableExtensionHeaderByType(IPProtocolId extensionType, int index)
{
    for (auto & elem : extensionHeaders)
        if ((elem)->getExtensionType() == extensionType) {
            if (index == 0)
                return elem;
            else
                index--;
        }
    return nullptr;
}

const Ipv6ExtensionHeader *Ipv6Header::findExtensionHeaderByType(IPProtocolId extensionType, int index) const
{
    for (const auto & elem : extensionHeaders)
        if ((elem)->getExtensionType() == extensionType) {
            if (index == 0)
                return elem;
            else
                index--;
        }
    return nullptr;
}

void Ipv6Header::setExtensionHeader(unsigned int k, const IPv6ExtensionHeaderPtr& extensionHeader_var)
{
    throw cRuntimeError(this, "setExtensionHeader() not supported, use addExtensionHeader()");
}

void Ipv6Header::addExtensionHeader(Ipv6ExtensionHeader *eh, int atPos)
{
    if (atPos != -1)
        throw cRuntimeError(this, "addExtensionHeader() does not support atPos parameter.");
    ASSERT((eh->getByteLength() >= 1) && (eh->getByteLength() % 8 == 0));
    int thisOrder = getExtensionHeaderOrder(eh);
    unsigned int i;
    for (i = 0; i < extensionHeaders.size(); i++) {
        int thatOrder = getExtensionHeaderOrder(extensionHeaders[i]);
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
    extensionHeaders.insert(extensionHeaders.begin() + i, eh);
}

/*
 *  Defines the order of extension headers according to RFC 2460 4.1.
 *  Note that Destination Options header may come both after the Hop-by-Hop
 *  extension and before the payload (if Routing presents).
 */
int Ipv6Header::getExtensionHeaderOrder(Ipv6ExtensionHeader *eh)
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

int Ipv6Header::calculateHeaderByteLength() const
{
    int len = 40;
    for (auto & elem : extensionHeaders)
        len += elem->getByteLength();
    return len;
}

/**
 * Note: it is assumed that headers are ordered as described in RFC 2460 4.1
 */
int Ipv6Header::calculateUnfragmentableHeaderByteLength() const
{
    int lastUnfragmentableExtensionIndex = -1;
    for (int i = ((int)extensionHeaders.size()) - 1; i >= 0; i--) {
        int type = extensionHeaders[i]->getExtensionType();
        if (type == IP_PROT_IPv6EXT_ROUTING || type == IP_PROT_IPv6EXT_HOP) {
            lastUnfragmentableExtensionIndex = i;
            break;
        }
    }

    int len = 40;
    for (int i = 0; i <= lastUnfragmentableExtensionIndex; i++)
        len += extensionHeaders[i]->getByteLength();
    return len;
}

/**
 * Note: it is assumed that headers are ordered as described in RFC 2460 4.1
 */
int Ipv6Header::calculateFragmentLength() const
{
    int len = B(getChunkLength()).get() - IPv6_HEADER_BYTES;
    unsigned int i;
    for (i = 0; i < extensionHeaders.size(); i++) {
        len -= extensionHeaders[i]->getByteLength();
        if (extensionHeaders[i]->getExtensionType() == IP_PROT_IPv6EXT_FRAGMENT)
            break;
    }
    ASSERT2(i < extensionHeaders.size(), "IPv6Datagram::calculateFragmentLength() called on non-fragment datagram");
    return len;
}

Ipv6ExtensionHeader *Ipv6Header::removeFirstExtensionHeader()
{
    handleChange();
    if (extensionHeaders.empty())
        return nullptr;
    Ipv6ExtensionHeader *eh = extensionHeaders.front();
    extensionHeaders.erase(extensionHeaders.begin());
    return eh;
}

Ipv6ExtensionHeader *Ipv6Header::removeExtensionHeader(IPProtocolId extensionType)
{
    handleChange();
    for (unsigned int i = 0; i < extensionHeaders.size(); i++) {
        if (extensionHeaders[i]->getExtensionType() == extensionType) {
            Ipv6ExtensionHeader *eh = extensionHeaders[i];
            extensionHeaders.erase(extensionHeaders.begin() + i);
            return eh;
        }
    }
    return nullptr;
}

Ipv6Header::~Ipv6Header()
{
    clean();
}

void Ipv6Header::clean()
{
    IPv6ExtensionHeaderPtr eh;

    while (!extensionHeaders.empty()) {
        eh = extensionHeaders.back();
        extensionHeaders.pop_back();    // remove pointer element from container
        delete eh;    // delete the header
    }
}

std::ostream& operator<<(std::ostream& out, const Ipv6ExtensionHeader& h)
{
    out << "{type:" << h.getExtensionType() << ",length:" << h.getByteLength() << "}";
    return out;
}

} // namespace inet

