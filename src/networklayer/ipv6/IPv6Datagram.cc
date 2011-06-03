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
        return (null = NULL);
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

IPv6ExtensionHeaderPtr IPv6Datagram::removeFirstExtensionHeader()
{
    static IPv6ExtensionHeaderPtr null;
    if ( extensionHeaders.size() == 0)
        return (null = NULL);
    IPv6ExtensionHeaderPtr eh = extensionHeaders.front();
    extensionHeaders.erase( extensionHeaders.begin() );
    return eh;
}

IPv6Datagram::~IPv6Datagram()
{
    IPv6ExtensionHeaderPtr eh;

    while ( ! extensionHeaders.empty() )
    {
        eh = extensionHeaders.back();
        extensionHeaders.pop_back(); // remove pointer element from container
        delete eh; // delete the header
    }
}
