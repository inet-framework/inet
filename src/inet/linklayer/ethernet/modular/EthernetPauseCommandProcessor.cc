//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ethernet/modular/EthernetPauseCommandProcessor.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"

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

