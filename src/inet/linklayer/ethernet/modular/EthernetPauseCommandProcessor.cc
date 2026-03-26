//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ethernet/modular/EthernetPauseCommandProcessor.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/common/SimulationContinuation.h"

namespace inet {

Define_Module(EthernetPauseCommandProcessor);

simsignal_t EthernetPauseCommandProcessor::pauseSentSignal = registerSignal("pauseSent");

void EthernetPauseCommandProcessor::initialize()
{
    outSink.reference(gate("out"), true);
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
    yieldBeforePush();
    outSink.pushPacket(check_and_cast<Packet *>(msg));
}

void EthernetPauseCommandProcessor::handleSendPause(Request *msg, Ieee802PauseCommand *etherctrl)
{
    MacAddress dest = etherctrl->getDestinationAddress();
    int pauseUnits = etherctrl->getPauseUnits();
    delete msg;

    EV_DETAIL << "Creating and sending PAUSE frame, with duration = " << pauseUnits << " units\n";

    // create Ethernet frame
    std::string framename = "pause-" + std::to_string(getId()) + "-" + std::to_string(seqNum++);
    auto packet = new Packet(framename.c_str());
    const auto& frame = makeShared<EthernetPauseFrame>();
    frame->setPauseTime(pauseUnits);
    packet->insertAtFront(frame);
    if (dest.isUnspecified())
        dest = MacAddress::MULTICAST_PAUSE_ADDRESS;
    packet->addTag<MacAddressReq>()->setDestAddress(dest);
    packet->addTag<PacketProtocolTag>()->setProtocol(&Protocol::ethernetFlowCtrl);

    EV_INFO << "Sending Pause command " << frame << " to lower layer.\n";
    emit(pauseSentSignal, pauseUnits);
    yieldBeforePush();
    outSink.pushPacket(packet);
}

void EthernetPauseCommandProcessor::pushPacket(Packet *packet, const cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    handleMessage(packet);
}

} // namespace inet

