//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
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


#include <omnetpp.h>
#include "UDPSink.h"
#include "UDPControlInfo_m.h"
#include "IPAddressResolver.h"


Define_Module(UDPSink);


void UDPSink::initialize()
{
    numReceived = 0;
    WATCH(numReceived);

    int port = par("localPort");
    if (port!=-1)
        bindToPort(port);
}

void UDPSink::handleMessage(cMessage *msg)
{
    processPacket(PK(msg));

    if (ev.isGUI())
    {
        char buf[32];
        sprintf(buf, "rcvd: %d pks", numReceived);
        getDisplayString().setTagArg("t",0,buf);
    }

}

void UDPSink::processPacket(cPacket *msg)
{
    EV << "Received packet: ";
    printPacket(msg);
    delete msg;

    numReceived++;
}

