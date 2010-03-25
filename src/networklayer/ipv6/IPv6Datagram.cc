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
#include "IPv6ExtensionHeaders_m.h"


Register_Class(IPv6Datagram);


std::ostream& operator<<(std::ostream& os, IPv6ExtensionHeaderPtr eh)
{
    return os << "(" << eh->getClassName() << ") " << eh->info();
}

IPv6Datagram& IPv6Datagram::operator=(const IPv6Datagram& other)
{
    IPv6Datagram_Base::operator=(other);

    for (ExtensionHeaders::const_iterator i=other.extensionHeaders.begin(); i!=other.extensionHeaders.end(); ++i)
        addExtensionHeader((*i)->dup());

    return *this;
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
        return (null=NULL);
    return extensionHeaders[k];
}

void IPv6Datagram::setExtensionHeader(unsigned int k, const IPv6ExtensionHeaderPtr& extensionHeader_var)
{
    throw cRuntimeError(this, "setExtensionHeader() not supported, use addExtensionHeader()");
}

void IPv6Datagram::addExtensionHeader(IPv6ExtensionHeader *eh, int atPos)
{
    if (atPos<0 || atPos>=(int)extensionHeaders.size())
    {
        extensionHeaders.push_back(eh);
        return;
    }

    // insert at position atPos, shift up the rest of the array
    extensionHeaders.insert(extensionHeaders.begin()+atPos, eh);
}

int IPv6Datagram::calculateHeaderByteLength() const
{
    int len = 40;
    for (unsigned int i=0; i<extensionHeaders.size(); i++)
        len += extensionHeaders[i]->getByteLength();
    return len;
}

//---

Register_Class(IPv6ExtensionHeader);


IPProtocolId IPv6ExtensionHeader::getExtensionType() const
{
    // FIXME msg files don't yet support readonly attrs that can be
    // redefined in subclasses, so for now we resort to the following
    // unsafe and unextensible nasty solution
    if (dynamic_cast<const IPv6HopByHopOptionsHeader*>(this)) {
        return IP_PROT_IPv6EXT_HOP;
    } else if (dynamic_cast<const IPv6RoutingHeader*>(this)) {
        return IP_PROT_IPv6EXT_ROUTING;
    } else if (dynamic_cast<const IPv6FragmentHeader*>(this)) {
        return IP_PROT_IPv6EXT_FRAGMENT;
    } else if (dynamic_cast<const IPv6DestinationOptionsHeader*>(this)) {
        return IP_PROT_IPv6EXT_DEST;
    } else if (dynamic_cast<const IPv6AuthenticationHeader*>(this)) {
        return IP_PROT_IPv6EXT_AUTH;
    } else if (dynamic_cast<const IPv6EncapsulatingSecurityPayloadHeader*>(this)) {
        return IP_PROT_IPv6EXT_ESP;
    } else {
        throw cRuntimeError("unrecognised HeaderExtension subclass %s in IPv6ExtensionHeader::getExtensionType()", getClassName());
    }
}

int IPv6ExtensionHeader::getByteLength() const
{
    // FIXME msg files don't yet support readonly attrs that can be
    // redefined in subclasses, so for now we resort to the following
    // unsafe and unextensible nasty solution
    if (dynamic_cast<const IPv6HopByHopOptionsHeader*>(this)) {
        return 8; // FIXME verify
    } else if (dynamic_cast<const IPv6RoutingHeader*>(this)) {
        return 8; // FIXME verify
    } else if (dynamic_cast<const IPv6FragmentHeader*>(this)) {
        return 8;
    } else if (dynamic_cast<const IPv6DestinationOptionsHeader*>(this)) {
        return 8; // FIXME verify
    } else if (dynamic_cast<const IPv6AuthenticationHeader*>(this)) {
        return 8; // FIXME verify
    } else if (dynamic_cast<const IPv6EncapsulatingSecurityPayloadHeader*>(this)) {
        return 8; // FIXME verify
    } else {
        throw cRuntimeError("unrecognised HeaderExtension subclass %s in IPv6ExtensionHeader::getExtensionType()", getClassName());
    }
}


