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

#include "TCPTunnelThread.h"

#include "AddressResolver.h"


Register_Class(TCPActiveTunnelThread);
Register_Class(TCPPassiveTunnelThread);

void TCPPassiveTunnelThread::realSocketAccept(cMessage *msg)
{
    int connSocket = msg->par("fd").longValue();
    check_and_cast<SocketsRTScheduler *>(simulation.getScheduler())->addSocket(this, NULL, connSocket, false);
    hostmod->createNewThreadFor(msg);
}

void TCPPassiveTunnelThread::initialize(int stage)
{
    TCPTunnelThreadBase::initialize(stage);

    if (stage == INITSTAGE_APPLICATION_LAYER)
    {
        realSocket = ::socket(AF_INET, SOCK_STREAM, 0);
        if (realSocket == INVALID_SOCKET)
            throw cRuntimeError("cannot create socket");

        int tunnelPort = par("tunnelPort").longValue();
        sockaddr_in sinInterface;
        sinInterface.sin_family = AF_INET;
        sinInterface.sin_addr.s_addr = INADDR_ANY;
        sinInterface.sin_port = htons(tunnelPort);
        if (bind(realSocket, (sockaddr*)&sinInterface, sizeof(sockaddr_in)) == SOCKET_ERROR)
            throw cRuntimeError("socket bind() failed");

        listen(realSocket, SOMAXCONN);

        SocketsRTScheduler *rtScheduler = check_and_cast<SocketsRTScheduler *>(simulation.getScheduler());
        rtScheduler->addSocket(this, NULL, realSocket, true);

        EV_INFO << "listening on localhost:" << tunnelPort << " port\n";
    }
}

