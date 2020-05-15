//
// Copyright (C) OpenSim Ltd.
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
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/protocol/ethernet/EthernetPauseCommandProcessor.h"

namespace inet {

Define_Module(EthernetPauseCommandProcessor);

simsignal_t EthernetPauseCommandProcessor::pauseSentSignal = registerSignal("pauseSent");

void EthernetPauseCommandProcessor::initialize()
{
    seqNum = 0;
}

void EthernetPauseCommandProcessor::handleMessage(cMessage *msg)
{
    if (auto rq = dynamic_cast<Request *>(msg)) {
        auto ctrl = msg->getControlInfo();
        if (auto cmd = dynamic_cast<Ieee802PauseCommand *>(ctrl)) {
            handleSendPause(rq, cmd);
            return;
        }
    }
    send(msg, "out");
}

void EthernetPauseCommandProcessor::handleSendPause(Request *msg, Ieee802PauseCommand *etherctrl)
{
    MacAddress dest = etherctrl->getDestinationAddress();
    int pauseUnits = etherctrl->getPauseUnits();
    delete msg;

    EV_DETAIL << "Creating and sending PAUSE frame, with duration = " << pauseUnits << " units\n";

    // create Ethernet frame
    char framename[40];
    sprintf(framename, "pause-%d-%d", getId(), seqNum++);
    auto packet = new Packet(framename);
    const auto& frame = makeShared<EthernetPauseFrame>();
    frame->setPauseTime(pauseUnits);
    packet->insertAtFront(frame);
    if (dest.isUnspecified())
        dest = MacAddress::MULTICAST_PAUSE_ADDRESS;
    packet->addTag<MacAddressReq>()->setDestAddress(dest);
    packet->addTag<PacketProtocolTag>()->setProtocol(&Protocol::ethernetFlowCtrl);

    EV_INFO << "Sending Pause command " << frame << " to lower layer.\n";
    send(packet, "out");

    emit(pauseSentSignal, pauseUnits);
}

} // namespace inet

