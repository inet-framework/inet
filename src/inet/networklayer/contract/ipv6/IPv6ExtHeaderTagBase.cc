//
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/networklayer/contract/ipv6/IPv6ExtHeaderTagBase.h"

#ifdef WITH_IPv6
#include "inet/networklayer/ipv6/Ipv6Header.h"
#endif // ifdef WITH_IPv6

namespace inet {

void Ipv6ExtHeaderTagBase::copy(const Ipv6ExtHeaderTagBase& other)
{
#ifdef WITH_IPv6
    for (const auto & elem : other.extensionHeaders)
        extensionHeaders.push_back((elem)->dup());
#endif // ifdef WITH_IPv6
}

Ipv6ExtHeaderTagBase& Ipv6ExtHeaderTagBase::operator=(const Ipv6ExtHeaderTagBase& other)
{
    if (this == &other)
        return *this;
    clean();
    Ipv6ExtHeaderTagBase_Base::operator=(other);
    copy(other);
    return *this;
}

void Ipv6ExtHeaderTagBase::clean()
{
#ifdef WITH_IPv6
    while (!extensionHeaders.empty()) {
        Ipv6ExtensionHeader *eh = extensionHeaders.back();
        extensionHeaders.pop_back();
        delete eh;
    }
#endif // ifdef WITH_IPv6
}

Ipv6ExtHeaderTagBase::~Ipv6ExtHeaderTagBase()
{
    clean();
}

unsigned int Ipv6ExtHeaderTagBase::getExtensionHeaderArraySize() const
{
    return extensionHeaders.size();
}

void Ipv6ExtHeaderTagBase::setExtensionHeaderArraySize(unsigned int size)
{
    throw cRuntimeError(this, "setExtensionHeaderArraySize() not supported, use addExtensionHeader()");
}

Ipv6ExtensionHeader *Ipv6ExtHeaderTagBase::getMutableExtensionHeader(unsigned int k)
{
    handleChange();
    ASSERT(k < extensionHeaders.size());
    return extensionHeaders[k];
}

const Ipv6ExtensionHeader *Ipv6ExtHeaderTagBase::getExtensionHeader(unsigned int k) const
{
    ASSERT(k < extensionHeaders.size());
    return extensionHeaders[k];
}

void Ipv6ExtHeaderTagBase::setExtensionHeader(unsigned int k, Ipv6ExtensionHeader *extensionHeader_var)
{
    throw cRuntimeError(this, "setExtensionHeader() not supported, use addExtensionHeader()");
}

void Ipv6ExtHeaderTagBase::addExtensionHeader(Ipv6ExtensionHeader *eh, int atPos)
{
#ifdef WITH_IPv6
    ASSERT(eh);
    if (atPos < 0 || (ExtensionHeaders::size_type)atPos >= extensionHeaders.size()) {
        extensionHeaders.push_back(eh);
        return;
    }

    // insert at position atPos, shift up the rest of the array
    extensionHeaders.insert(extensionHeaders.begin() + atPos, eh);
#else // ifdef WITH_IPv6
    throw cRuntimeError(this, "INET was compiled without IPv6 support");
#endif // ifdef WITH_IPv6
}

Ipv6ExtensionHeader *Ipv6ExtHeaderTagBase::removeFirstExtensionHeader()
{
    if (extensionHeaders.empty())
        return nullptr;

#ifdef WITH_IPv6
    auto first = extensionHeaders.begin();
    Ipv6ExtensionHeader *ret = *first;
    extensionHeaders.erase(first);
    return ret;
#else // ifdef WITH_IPv6
    throw cRuntimeError(this, "INET was compiled without IPv6 support");
#endif // ifdef WITH_IPv6
}

} // namespace inet

