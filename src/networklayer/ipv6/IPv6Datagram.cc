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


#include "IPv6Datagram.h"
#include "IPv6ExtensionHeaders.h"


Register_Class(IPv6Datagram);

std::ostream& operator<<(std::ostream& os, IPv6ExtensionHeaderPtr eh)
{
    return os << "(" << eh->getClassName() << ") " << eh->info();
}

IPv6Datagram& IPv6Datagram::operator=(const IPv6Datagram& other)
{
    if (this==&other) return *this;
    clean();
    IPv6Datagram_Base::operator=(other);
    copy(other);
    return *this;
}

void IPv6Datagram::copy(const IPv6Datagram& other)
{
    for (ExtensionHeaders::const_iterator i=other.extensionHeaders.begin(); i!=other.extensionHeaders.end(); ++i)
        addExtensionHeader((*i)->dup());
}

void IPv6Datagram::setExtensionHeaderArraySize(unsigned int size)
{
    throw cRuntimeError(this, "setExtensionHeaderArraySize() not supported, use addExtensionHeader()");
}

unsigned int IPv6Datagram::getExtensionHeaderArraySize() const
{
    return extensionHeaders.size();
}

IPv6ExtensionHeaderPtr& IPv6Datagram::getExtensionHeader(unsigned int k)
{
    static IPv6ExtensionHeaderPtr null;
    if (k>=extensionHeaders.size())
        return (null = NULL);
    return extensionHeaders[k];
}

IPv6ExtensionHeader* IPv6Datagram::findExtensionHeaderByType(IPProtocolId extensionType, int index) const
{
	for (ExtensionHeaders::const_iterator it=extensionHeaders.begin(); it != extensionHeaders.end(); ++it)
		if ((*it)->getExtensionType() == extensionType)
		{
			if (index == 0)
				return *it;
			else
				index--;
		}
	return NULL;
}


void IPv6Datagram::setExtensionHeader(unsigned int k, const IPv6ExtensionHeaderPtr& extensionHeader_var)
{
    throw cRuntimeError(this, "setExtensionHeader() not supported, use addExtensionHeader()");
}

void IPv6Datagram::addExtensionHeader(IPv6ExtensionHeader *eh, int atPos)
{
	if (atPos != -1)
		throw cRuntimeError(this, "addExtensionHeader() does not support atPos parameter.");

	int thisOrder = getExtensionHeaderOrder(eh);
    unsigned int i;
	for (i=0; i<extensionHeaders.size(); i++)
    {
        int thatOrder = getExtensionHeaderOrder(extensionHeaders[i]);
        if (thisOrder != -1 && thatOrder > thisOrder)
            break;
        else if (thisOrder == thatOrder)
        {
            if (thisOrder == 1)
                thisOrder=6;
            else if (thisOrder != -1)
                throw cRuntimeError(this, "addExtensionHeader() duplicate extension header: %d",
                                        eh->getExtensionType());
        }
    }

	// insert at position atPos, shift up the rest of the array
    extensionHeaders.insert(extensionHeaders.begin()+i, eh);
}

/*
 *  Defines the order of extension headers according to RFC 2460 4.1.
 *  Note that Destination Options header may come both after the Hop-by-Hop
 *  extension and before the payload (if Routing presents).
 */
int IPv6Datagram::getExtensionHeaderOrder(IPv6ExtensionHeader *eh)
{
	switch (eh->getExtensionType())
	{
	case IP_PROT_IPv6EXT_HOP: return 0;
	case IP_PROT_IPv6EXT_DEST: return 1;
	case IP_PROT_IPv6EXT_ROUTING: return 2;
	case IP_PROT_IPv6EXT_FRAGMENT: return 3;
	case IP_PROT_IPv6EXT_AUTH: return 4;
	case IP_PROT_IPv6EXT_ESP: return 5;
	// second IP_PROT_IPv6EXT_DEST has order 6
	case IP_PROT_IPv6EXT_MOB: return 7;
	default: return -1;
	}
}

int IPv6Datagram::calculateHeaderByteLength() const
{
    int len = 40;
    for (unsigned int i=0; i<extensionHeaders.size(); i++)
        len += extensionHeaders[i]->getByteLength();
    return len;
}

/**
 * Note: it is assumed that headers are ordered as described in RFC 2460 4.1
 */
int IPv6Datagram::calculateUnfragmentableHeaderByteLength() const
{
	int lastUnfragmentableExtensionIndex = -1;
	for (int i=((int)extensionHeaders.size())-1; i>=0; i--)
	{
		int type = extensionHeaders[i]->getExtensionType();
		if (type == IP_PROT_IPv6EXT_ROUTING || type == IP_PROT_IPv6EXT_HOP)
		{
			lastUnfragmentableExtensionIndex = i;
			break;
		}
	}

	int len = 40;
	for (int i=0; i<=lastUnfragmentableExtensionIndex; i++)
		len += extensionHeaders[i]->getByteLength();
	return len;
}

/**
 * Note: it is assumed that headers are ordered as described in RFC 2460 4.1
 */
int IPv6Datagram::calculateFragmentLength() const
{
	int len = getByteLength() - IPv6_HEADER_BYTES;
	unsigned int i;
	for (i=0; i<extensionHeaders.size(); i++)
	{
		len -= extensionHeaders[i]->getByteLength();
        if (extensionHeaders[i]->getExtensionType() == IP_PROT_IPv6EXT_FRAGMENT)
            break;
	}
	ASSERT2(i<extensionHeaders.size(), "IPv6Datagram::calculateFragmentLength() called on non-fragment datagram");
	return len;
}

IPv6ExtensionHeader* IPv6Datagram::removeFirstExtensionHeader()
{
    if (extensionHeaders.empty())
        return NULL;
    IPv6ExtensionHeader* eh = extensionHeaders.front();
    extensionHeaders.erase( extensionHeaders.begin() );
    return eh;
}

IPv6ExtensionHeader* IPv6Datagram::removeExtensionHeader(IPProtocolId extensionType)
{
	for (unsigned int i=0; i<extensionHeaders.size(); i++)
	{
		if (extensionHeaders[i]->getExtensionType() == extensionType)
		{
			IPv6ExtensionHeader* eh = extensionHeaders[i];
			extensionHeaders.erase(extensionHeaders.begin()+i);
			return eh;
		}
	}
	return NULL;
}


IPv6Datagram::~IPv6Datagram()
{
    clean();
}

void IPv6Datagram::clean()
{
    IPv6ExtensionHeaderPtr eh;

    while ( ! extensionHeaders.empty() )
    {
        eh = extensionHeaders.back();
        extensionHeaders.pop_back(); // remove pointer element from container
        delete eh; // delete the header
    }
}
