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


#include "UDPSink.h"
#include "UDPControlInfo_m.h"


Define_Module(UDPSink);

simsignal_t UDPSink::rcvdPkSignal = registerSignal("rcvdPk");


void UDPSink::initialize(int stage)
{
    AppBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL)
    {
        numReceived = 0;
        WATCH(numReceived);
    }
}

void UDPSink::handleMessageWhenUp(cMessage *msg)
{
    if (msg->getKind() == UDP_I_DATA)
    {
        // process incoming packet
        processPacket(PK(msg));
    }
    else if (msg->getKind() == UDP_I_ERROR)
    {
        EV << "Ignoring UDP error report\n";
        delete msg;
    }
    else
    {
        error("Unrecognized message (%s)%s", msg->getClassName(), msg->getName());
    }

    if (ev.isGUI())
    {
        char buf[32];
        sprintf(buf, "rcvd: %d pks", numReceived);
        getDisplayString().setTagArg("t", 0, buf);
    }
}

void UDPSink::finish()
{
    AppBase::finish();
    EV << getFullPath() << ": received " << numReceived << " packets\n";
}

void UDPSink::processPacket(cPacket *pk)
{
    EV << "Received packet: " << UDPSocket::getReceivedPacketInfo(pk) << endl;
    emit(rcvdPkSignal, pk);
    delete pk;

    numReceived++;
}

bool UDPSink::startApp(IDoneCallback *doneCallback)
{
    socket.setOutputGate(gate("udpOut"));
    int localPort = par("localPort");
    socket.bind(localPort);
    socket.joinLocalMulticastGroups();
    return true;
}

bool UDPSink::stopApp(IDoneCallback *doneCallback)
{
    //TODO if(socket.isOpened()) socket.close();
    return true;
}

bool UDPSink::crashApp(IDoneCallback *doneCallback)
{
    return true;
}

