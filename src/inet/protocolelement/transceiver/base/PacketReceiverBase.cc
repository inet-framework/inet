//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
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

