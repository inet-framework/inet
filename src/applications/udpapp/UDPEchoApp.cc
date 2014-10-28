//
// Copyright (C) 2011 Andras Varga
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


#include "UDPEchoApp.h"
#include "UDPControlInfo_m.h"


Define_Module(UDPEchoApp);

simsignal_t UDPEchoApp::pkSignal = registerSignal("pk");


void UDPEchoApp::initialize(int stage)
{
    ApplicationBase::initialize(stage);

    if (stage == 0)
    {
        // init statistics
        numEchoed = 0;
        WATCH(numEchoed);
    }
    else if (stage == 3)
    {
        if (ev.isGUI())
            updateDisplay();
    }
}

void UDPEchoApp::handleMessageWhenUp(cMessage *msg)
{
    if (msg->getKind() == UDP_I_ERROR)
    {
        // ICMP error report -- discard it
        delete msg;
    }
    else if (msg->getKind() == UDP_I_DATA)
    {
        cPacket *pk = PK(msg);
        // statistics
        numEchoed++;
        emit(pkSignal, pk);

        // determine its source address/port
        UDPDataIndication *ctrl = check_and_cast<UDPDataIndication *>(pk->removeControlInfo());
        IPvXAddress srcAddress = ctrl->getSrcAddr();
        int srcPort = ctrl->getSrcPort();
        delete ctrl;

        // send back
        socket.sendTo(pk, srcAddress, srcPort);

        if (ev.isGUI())
            updateDisplay();
    }
    else
    {
        throw cRuntimeError("Message received with unexpected message kind = %d", msg->getKind());
    }
}

void UDPEchoApp::updateDisplay()
{
    char buf[40];
    sprintf(buf, "echoed: %d pks", numEchoed);
    getDisplayString().setTagArg("t", 0, buf);
}

void UDPEchoApp::finish()
{
    ApplicationBase::finish();
}

bool UDPEchoApp::handleNodeStart(IDoneCallback *doneCallback)
{
    socket.setOutputGate(gate("udpOut"));
    int localPort = par("localPort");
    socket.bind(localPort);
    socket.joinLocalMulticastGroups();
    return true;
}

bool UDPEchoApp::handleNodeShutdown(IDoneCallback *doneCallback)
{
    //TODO if(socket.isOpened()) socket.close();
    return true;
}

void UDPEchoApp::handleNodeCrash()
{
}

