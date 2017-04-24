//
// Copyright (C) 2014 Irene Ruengeler
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/contract/NetworkHeaderBase.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/applications/tunapp/TunLoopbackApp.h"
#include "inet/transportlayer/contract/TransportHeaderBase_m.h"

namespace inet {

Define_Module(TunLoopbackApp);

simsignal_t TunLoopbackApp::sentPkSignal = registerSignal("sentPk");
simsignal_t TunLoopbackApp::rcvdPkSignal = registerSignal("rcvdPk");

TunLoopbackApp::TunLoopbackApp() :
    packetsSent(0),
    packetsReceived(0)
{
}

TunLoopbackApp::~TunLoopbackApp()
{
}

void TunLoopbackApp::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        tunInterface = par("tunInterface");
        packetsSent = 0;
        packetsReceived = 0;
        IInterfaceTable *interfaceTable = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        InterfaceEntry *interfaceEntry = interfaceTable->getInterfaceByName(tunInterface);
        if (interfaceEntry == nullptr)
            throw cRuntimeError("TUN interface not found: %s", tunInterface);
        tunSocket.setOutputGate(gate("socketOut"));
        tunSocket.open(interfaceEntry->getInterfaceId());
    }
}

void TunLoopbackApp::handleMessage(cMessage *message)
{
    if (message->arrivedOn("socketIn")) {
        EV_INFO << "Message " << message->getName() << " arrived from tun. " << packetsReceived + 1 << " packets received so far\n";
        packetsReceived++;

        //FIXME KLUDGE next lines obsoleted
        NetworkHeaderBase *networkDatagram = check_and_cast<NetworkHeaderBase *>(message);
        TransportHeaderBase *transportPacket = check_and_cast<TransportHeaderBase *>(check_and_cast<cPacket *>(message)->getEncapsulatedPacket());

        transportPacket->setDestinationPort(transportPacket->getSourcePort());
        transportPacket->setSourcePort(transportPacket->getDestinationPort());
        networkDatagram->setSourceAddress(networkDatagram->getDestinationAddress());
        networkDatagram->setDestinationAddress(networkDatagram->getSourceAddress());
        delete message->removeControlInfo();
        message->clearTags();
        tunSocket.send(check_and_cast<Packet*>(message));
        packetsSent++;
    }
    else
        throw cRuntimeError("Unknown message: %s", message->getName());
}


void TunLoopbackApp::finish()
{
    EV_INFO << "packets sent: " << packetsSent << endl;
    EV_INFO << "packets received: " << packetsReceived << endl;
}

} // namespace inet
