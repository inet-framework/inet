//
// Copyright (C) 2010 Zoltan Bojthe
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

#include "inet/networklayer/ipv6/IPv6ExtensionHeaders.h"

namespace inet {

void IPv6RoutingHeader::setAddressArraySize(unsigned int size)
{
    IPv6RoutingHeader_Base::setAddressArraySize(size);
    byteLength_var = 8 + 16 * size;
}

void IPv6HopByHopOptionsHeader::clean()
{
    // clean away the options
    for (int i = getOptionsArraySize()-1; i >= 0; --i)
    {
        delete getOptions(i);
    }
}

void IPv6HopByHopOptionsHeader::copy(const IPv6HopByHopOptionsHeader& other)
{
    if (getOptionsArraySize() > 0)
    {
        clean();
    }
    setOptionsArraySize(other.getOptionsArraySize());
    for (int i = other.getOptionsArraySize()-1; i >= 0; --i)
    {
        setOptions(i, other.getOptions(i)->dup());
    }
}

IPv6HopByHopOptionsHeader::~IPv6HopByHopOptionsHeader()
{
    clean();
}

IPv6HopByHopOptionsHeader::IPv6HopByHopOptionsHeader(const IPv6HopByHopOptionsHeader &other)
{
    setOptionsArraySize(0);
    copy(other);
}

IPv6HopByHopOptionsHeader& IPv6HopByHopOptionsHeader::operator=(const IPv6HopByHopOptionsHeader& other)
{
    copy(other);
    return *this;
}

} // namespace inet

