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

#include "inet/applications/udpapp/UdpEchoApp.h"

#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/transportlayer/common/L4PortTag_m.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"

namespace inet {

Define_Module(UdpEchoApp);

void UdpEchoApp::initialize(int stage)
{
    ApplicationBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        // init statistics
        numEchoed = 0;
        WATCH(numEchoed);
    }
}

void UdpEchoApp::handleMessageWhenUp(cMessage *msg)
{
    if (msg->getKind() == UDP_I_ERROR) {
        // ICMP error report -- discard it
        delete msg;
    }
    else if (msg->getKind() == UDP_I_DATA) {
        Packet *pk = check_and_cast<Packet *>(msg);

        // determine its source address/port
        L3Address remoteAddress = pk->getTag<L3AddressInd>()->getSrcAddress();
        int srcPort = pk->getTag<L4PortInd>()->getSrcPort();
        pk->clearTags();
        pk->trim();

        // statistics
        numEchoed++;
        emit(packetSentSignal, pk);
        // send back
        socket.sendTo(pk, remoteAddress, srcPort);
    }
    else {
        throw cRuntimeError("Message received with unexpected message kind = %d", msg->getKind());
    }
}

void UdpEchoApp::refreshDisplay() const
{
    char buf[40];
    sprintf(buf, "echoed: %d pks", numEchoed);
    getDisplayString().setTagArg("t", 0, buf);
}

void UdpEchoApp::finish()
{
    ApplicationBase::finish();
}

bool UdpEchoApp::handleNodeStart(IDoneCallback *doneCallback)
{
    socket.setOutputGate(gate("socketOut"));
    int localPort = par("localPort");
    socket.bind(localPort);
    MulticastGroupList mgl = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this)->collectMulticastGroups();
    socket.joinLocalMulticastGroups(mgl);
    return true;
}

bool UdpEchoApp::handleNodeShutdown(IDoneCallback *doneCallback)
{
    //TODO if(socket.isOpened()) socket.close();
    return true;
}

void UdpEchoApp::handleNodeCrash()
{
}

} // namespace inet

