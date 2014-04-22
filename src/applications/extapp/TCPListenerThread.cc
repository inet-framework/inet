//
// Copyright (C) 2014 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//
// author: Zoltan Bojthe

#include "TCPListenerThread.h"

#include "AddressResolver.h"


Define_Module(TCPListenerThread);

void TCPListenerThread::initialize(int stage)
{
    TCPThreadBase::initialize(stage);

    if (stage == INITSTAGE_APPLICATION_LAYER)
    {
        const char *localAddress = par("localAddress");
        int localPort = par("localPort");
        Address localAddr;
        if (localAddress[0])
            localAddr = AddressResolver().resolve(localAddress);
        socket.readDataTransferModePar(*this);
        socket.bind(localAddress[0] ? Address(localAddress) : Address(), localPort);
        socket.listen();
        EV_INFO << "listening on " << localAddr << ":" << localPort << " port\n";
    }
}

