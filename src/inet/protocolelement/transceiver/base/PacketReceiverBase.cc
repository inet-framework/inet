//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/transceiver/base/PacketReceiverBase.h"

#include "inet/common/DirectionTag_m.h"
#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/common/InterfaceTag_m.h"

namespace inet {

PacketReceiverBase::~PacketReceiverBase()
{
    delete rxSignal;
    rxSignal = nullptr;
}

void PacketReceiverBase::initialize(int stage)
{
    OperationalMixin::initialize(stage);
    PacketProcessorBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        rxDatarate = bps(par("datarate"));
        inputGate = gate("in");
        outputGate = gate("out");
        networkInterface = findContainingNicModule(this);
        consumer = findConnectedModule<IPassivePacketSink>(outputGate);
    }
    else if (stage == INITSTAGE_QUEUEING)
        checkPacketOperationSupport(outputGate);
}

void PacketReceiverBase::handleMessageWhenUp(cMessage *message)
{
    throw cRuntimeError("Invalid operation");
}

void PacketReceiverBase::handleMessageWhenDown(cMessage *msg)
{
    if (!msg->isSelfMessage()) {
        EV << "Dropping message because interface is down" << EV_FIELD(msg) << EV_ENDL;
        delete msg;
    }
    else
        OperationalMixin::handleMessageWhenDown(msg);
}

void PacketReceiverBase::handleStartOperation(LifecycleOperation *operation)
{
}

void PacketReceiverBase::handleStopOperation(LifecycleOperation *operation)
{
    delete rxSignal;
    rxSignal = nullptr;
}

void PacketReceiverBase::handleCrashOperation(LifecycleOperation *operation)
{
    delete rxSignal;
    rxSignal = nullptr;
}

Packet *PacketReceiverBase::decodePacket(Signal *signal) const
{
    auto packet = check_and_cast<Packet *>(signal->decapsulate());
    // TODO check signal physical properties such as datarate, modulation, etc.
    packet->addTagIfAbsent<DirectionTag>()->setDirection(DIRECTION_INBOUND);
    if (networkInterface != nullptr)
        packet->addTag<InterfaceInd>()->setInterfaceId(networkInterface->getInterfaceId());
    packet->setBitError(signal->hasBitError());
    return packet;
}

} // namespace inet

