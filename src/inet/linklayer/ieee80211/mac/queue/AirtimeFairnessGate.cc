//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/queue/AirtimeFairnessGate.h"

#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/common/Ieee80211AirtimeInd.h"
#include "inet/networklayer/common/NetworkInterface.h"

namespace inet {
namespace ieee80211 {

Define_Module(AirtimeFairnessGate);

simsignal_t AirtimeFairnessGate::deficitChangedSignal = cComponent::registerSignal("deficitChanged");

void AirtimeFairnessGate::initialize(int stage)
{
    PacketGateBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        quantum = par("quantum");
        weight = par("weight");
        fairnessEnabled = par("fairnessEnabled");
        if (quantum <= SIMTIME_ZERO)
            throw cRuntimeError("The quantum parameter must be positive");
        if (weight <= 0)
            throw cRuntimeError("The weight parameter must be positive");
        frameTransmittedAirtimeSignal = registerSignal("frameTransmittedAirtime");
        isOpen_ = isEligible(); // deficit starts at zero -> eligible -> open
        WATCH(stationAddress);
        WATCH(deficit);
    }
    else if (stage == INITSTAGE_LINK_LAYER)
        // The coordination function (Dcf/Hcf) emits frameTransmittedAirtime from within the
        // containing network interface; subscribing there scopes the accounting to this
        // interface's transmissions (the frame's receiver disambiguates the stations).
        getContainingNicModule(this)->subscribe(frameTransmittedAirtimeSignal, this);
}

void AirtimeFairnessGate::processPacket(Packet *packet)
{
    PacketGateBase::processPacket(packet);
    if (stationAddress.isUnspecified()) {
        const auto& header = packet->peekAtFront<Ieee80211MacHeader>();
        stationAddress = header->getReceiverAddress();
    }
}

void AirtimeFairnessGate::setDeficit(simtime_t value)
{
    if (deficit != value) {
        deficit = value;
        emit(deficitChangedSignal, deficit.dbl());
    }
}

void AirtimeFairnessGate::addQuantum()
{
    setDeficit(deficit + quantum * weight);
    updateGateState();
}

void AirtimeFairnessGate::updateGateState()
{
    bool eligible = isEligible();
    if (eligible && isClosed())
        open();
    else if (!eligible && isOpen())
        close();
}

bool AirtimeFairnessGate::isBacklogged() const
{
    return provider != nullptr && provider.canPullSomePacket();
}

Packet *AirtimeFairnessGate::peekPacket() const
{
    return provider != nullptr ? provider.canPullPacket() : nullptr;
}

void AirtimeFairnessGate::handleCanPullPacketChanged(const cGate *gate)
{
    Enter_Method("handleCanPullPacketChanged");
    // Forward even while the gate is closed: a station can become backlogged while its
    // deficit is negative (gate shut), and the scheduler must still learn about it so it
    // can top the station up and eventually serve it. PacketGateBase would swallow this
    // notification while the gate is closed.
    if (collector != nullptr)
        collector.handleCanPullPacketChanged();
}

int AirtimeFairnessGate::getNumPackets() const
{
    return queueing::PacketFlowBase::getNumPackets();
}

b AirtimeFairnessGate::getTotalLength() const
{
    return queueing::PacketFlowBase::getTotalLength();
}

Packet *AirtimeFairnessGate::getPacket(int index) const
{
    return queueing::PacketFlowBase::getPacket(index);
}

bool AirtimeFairnessGate::isEmpty() const
{
    return queueing::PacketFlowBase::isEmpty();
}

void AirtimeFairnessGate::removePacket(Packet *packet)
{
    queueing::PacketFlowBase::removePacket(packet);
}

void AirtimeFairnessGate::removeAllPackets()
{
    queueing::PacketFlowBase::removeAllPackets();
}

void AirtimeFairnessGate::receiveSignal(cComponent *source, simsignal_t signalID, cObject *object, cObject *details)
{
    if (signalID == frameTransmittedAirtimeSignal) {
        Enter_Method("%s", cComponent::getSignalName(signalID));
        auto info = check_and_cast<Ieee80211AirtimeInd *>(object);
        // charge only frames sent to the station this gate serves
        if (!stationAddress.isUnspecified() && info->receiverAddress == stationAddress) {
            setDeficit(deficit - info->airtime);
            EV_DEBUG << "Charged " << info->airtime << " airtime to " << stationAddress
                     << ", deficit now " << deficit << EV_ENDL;
            updateGateState(); // may close the gate if the deficit went negative
        }
    }
}

} // namespace ieee80211
} // namespace inet
