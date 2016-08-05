//
// Copyright (C) 2016 OpenSim Ltd
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
// @authors Andras Varrga, Zoltan Bojthe
// based on IPv4ControlInfo.cc

#include "inet/networklayer/common/OrigNetworkDatagramTag.h"

namespace inet {

OrigNetworkDatagramInd::~OrigNetworkDatagramInd()
{
    clean();
}

OrigNetworkDatagramInd& OrigNetworkDatagramInd::operator=(const OrigNetworkDatagramInd& other)
{
    if (this == &other)
        return *this;
    clean();
    OrigNetworkDatagramInd_Base::operator=(other);
    copy(other);
    return *this;
}

void OrigNetworkDatagramInd::clean()
{
    if (dgram != nullptr)
        dropAndDelete(dgram);
}

void OrigNetworkDatagramInd::copy(const OrigNetworkDatagramInd& other)
{
    dgram = other.dgram;

    if (dgram) {
        dgram = check_and_cast<cPacket*>(dgram)->dup();
        take(dgram);
    }
}

void OrigNetworkDatagramInd::setOrigDatagram(cPacket *d)
{
    if (dgram)
        throw cRuntimeError(this, "OrigNetworkDatagramInd::setOrigDatagram(): a datagram is already attached");
    dgram = d;
    take(dgram);
}

cPacket *OrigNetworkDatagramInd::removeOrigDatagram()
{
    if (!dgram)
        throw cRuntimeError(this, "OrigNetworkDatagramInd::removeOrigDatagram(): no datagram attached "
                                  "(already removed, or maybe this OrigNetworkDatagramInd does not come "
                                  "from the IPvX module?)");

    cPacket *ret = dgram;
    drop(dgram);
    dgram = nullptr;
    return ret;
}

} // namespace inet

