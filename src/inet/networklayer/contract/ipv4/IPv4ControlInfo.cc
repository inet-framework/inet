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

#include "inet/networklayer/contract/ipv4/IPv4ControlInfo.h"

#ifdef WITH_IPv4
#include "inet/networklayer/ipv4/IPv4Datagram.h"
#endif // ifdef WITH_IPv4

namespace inet {

IPv4ControlInfo::~IPv4ControlInfo()
{
    clean();
}

IPv4ControlInfo& IPv4ControlInfo::operator=(const IPv4ControlInfo& other)
{
    if (this == &other)
        return *this;
    clean();
    IPv4ControlInfo_Base::operator=(other);
    copy(other);
    return *this;
}

void IPv4ControlInfo::clean()
{
#ifdef WITH_IPv4
    dropAndDelete(dgram);
#else // ifdef WITH_IPv4
    if (dgram)
        throw cRuntimeError("INET was compiled without IPv4 support");
#endif // ifdef WITH_IPv4
}

void IPv4ControlInfo::copy(const IPv4ControlInfo& other)
{
    dgram = other.dgram;

    if (dgram) {
#ifdef WITH_IPv4
        dgram = dgram->dup();
        take(dgram);
#else // ifdef WITH_IPv4
        throw cRuntimeError(this, "INET was compiled without IPv4 support");
#endif // ifdef WITH_IPv4
    }
}

void IPv4ControlInfo::setOrigDatagram(IPv4Datagram *d)
{
#ifdef WITH_IPv4
    if (dgram)
        throw cRuntimeError(this, "IPv4ControlInfo::setOrigDatagram(): a datagram is already attached");
    dgram = d;
    take(dgram);
#else // ifdef WITH_IPv4
    throw cRuntimeError(this, "INET was compiled without IPv4 support");
#endif // ifdef WITH_IPv4
}

IPv4Datagram *IPv4ControlInfo::removeOrigDatagram()
{
#ifdef WITH_IPv4
    if (!dgram)
        throw cRuntimeError(this, "IPv4ControlInfo::removeOrigDatagram(): no datagram attached "
                                  "(already removed, or maybe this IPv4ControlInfo does not come "
                                  "from the IPv4 module?)");

    IPv4Datagram *ret = dgram;
    drop(dgram);
    dgram = nullptr;
    return ret;
#else // ifdef WITH_IPv4
    throw cRuntimeError(this, "INET was compiled without IPv4 support");
#endif // ifdef WITH_IPv4
}

} // namespace inet

