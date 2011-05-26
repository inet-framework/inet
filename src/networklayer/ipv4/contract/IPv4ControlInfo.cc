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

#include "IPv4ControlInfo.h"
#include "IPv4Datagram.h"

IPv4ControlInfo::~IPv4ControlInfo()
{
    if (dgram)
    {
        drop(dgram);
        delete dgram;
    }
}

void IPv4ControlInfo::setOrigDatagram(IPv4Datagram *d)
{
    if (dgram)
        throw cRuntimeError(this, "IPv4ControlInfo::setOrigDatagram(): a datagram is already attached");

    dgram = d;
    take(dgram);
}

IPv4Datagram *IPv4ControlInfo::removeOrigDatagram()
{
    if (!dgram)
        throw cRuntimeError(this, "IPv4ControlInfo::removeOrigDatagram(): no datagram attached "
                  "(already removed, or maybe this IPv4ControlInfo does not come "
                  "from the IPv4 module?)");

    IPv4Datagram *ret = dgram;
    drop(dgram);
    dgram = NULL;
    return ret;
}


