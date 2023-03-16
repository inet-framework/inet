//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/base/PacketGateBase.h"

namespace inet {
namespace queueing {

simsignal_t PacketGateBase::gateStateChangedSignal = registerSignal("gateStateChanged");

void PacketGateBase::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        bitrate = bps(par("bitrate"));
        extraLength = b(par("extraLength"));
        extraDuration = par("extraDuration");
        getDisplayString().setTagArg("i", 2, 20);
        WATCH(isOpen_);
    }
    else if (stage == INITSTAGE_LAST)
        emit(gateStateChangedSignal, isOpen_);
}

void PacketGateBase::finish()
{
    emit(gateStateChangedSignal, isOpen_);
}

cGate *PacketGateBase::getRegistrationForwardingGate(cGate *gate)
{
    if (gate == outputGate)
        return inputGate;
    else if (gate == inputGate)
        return outputGate;
    else
        throw cRuntimeError("Unknown gate");
}

void PacketGateBase::open()
{
    ASSERT(!isOpen_);
    EV_DEBUG << "Opening gate" << EV_ENDL;
    isOpen_ = true;
    if (producer != nullptr)
        producer->handleCanPushPacketChanged(inputGate->getPathStartGate());
    if (collector != nullptr)
        collector->handleCanPullPacketChanged(outputGate->getPathEndGate());
    emit(gateStateChangedSignal, isOpen_);
    updateDisplayString();
}

void PacketGateBase::close()
{
    ASSERT(isOpen_);
    EV_DEBUG << "Closing gate" << EV_ENDL;
    isOpen_ = false;
    if (producer != nullptr)
        producer->handleCanPushPacketChanged(inputGate->getPathStartGate());
    if (collector != nullptr)
        collector->handleCanPullPacketChanged(outputGate->getPathEndGate());
    emit(gateStateChangedSignal, isOpen_);
    updateDisplayString();
}

void PacketGateBase::processPacket(Packet *packet)
{
    if (!isOpen_)
        throw cRuntimeError("Illegal operation on the gate, a packet is being passed through while the gate is not open");
    EV_INFO << "Passing through packet" << EV_FIELD(packet) << EV_ENDL;
}

bool PacketGateBase::canPushSomePacket(cGate *gate) const
{
    return isOpen_ && canPacketFlowThrough(nullptr) && PacketFlowBase::canPushSomePacket(gate);
}

bool PacketGateBase::canPushPacket(Packet *packet, cGate *gate) const
{
    return isOpen_ && canPacketFlowThrough(packet) && PacketFlowBase::canPushPacket(packet, gate);
}

bool PacketGateBase::canPullSomePacket(cGate *gate) const
{
    auto packet = PacketFlowBase::canPullPacket(gate);
    return isOpen_ && canPacketFlowThrough(packet) && PacketFlowBase::canPullSomePacket(gate);
}

Packet *PacketGateBase::canPullPacket(cGate *gate) const
{
    auto packet = PacketFlowBase::canPullPacket(gate);
    return isOpen_ && canPacketFlowThrough(packet) ? packet : nullptr;
}

void PacketGateBase::handleCanPushPacketChanged(cGate *gate)
{
    Enter_Method("handleCanPushPacketChanged");
    if (isOpen_)
        PacketFlowBase::handleCanPushPacketChanged(gate);
}

void PacketGateBase::handleCanPullPacketChanged(cGate *gate)
{
    Enter_Method("handleCanPullPacketChanged");
    if (isOpen_)
        PacketFlowBase::handleCanPullPacketChanged(gate);
}

bool PacketGateBase::canPacketFlowThrough(Packet *packet) const
{
    ASSERT(isOpen_);
    return true;
}

void PacketGateBase::updateDisplayString() const
{
    PacketFlowBase::updateDisplayString();
    getDisplayString().setTagArg("i", 1, isOpen_ ? "green" : "red");
    getDisplayString().setTagArg("i", 2, 50);
}

} // namespace queueing
} // namespace inet

