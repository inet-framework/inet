//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/lifetime/CarrierBasedLifeTimer.h"

#include <algorithm>

#include "inet/common/ModuleAccess.h"
#include "inet/common/Simsignals.h"

namespace inet {

Define_Module(CarrierBasedLifeTimer);

void CarrierBasedLifeTimer::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        networkInterface = getContainingNicModule(this);
        networkInterface->subscribe(interfaceStateChangedSignal, this);
        packetCollection.reference(this, "collectionModule", true);
        auto packetCollectionModule = check_and_cast<cModule *>(packetCollection.get());
        packetCollectionModule->subscribe(packetPushedSignal, this);
    }
}

void CarrierBasedLifeTimer::clearCollection()
{
    while (!packetCollection->isEmpty()) {
        auto packet = packetCollection->getPacket(0);
        packetCollection->removePacket(packet);
        EV_WARN << "Dropping packet because interface has no carrier" << EV_FIELD(packet) << EV_ENDL;
        PacketDropDetails details;
        details.setReason(INTERFACE_DOWN);
        emit(packetDroppedSignal, packet, &details);
        delete packet;
    }
}

void CarrierBasedLifeTimer::receiveSignal(cComponent *source, simsignal_t signal, cObject *obj, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signal));

    if (signal == interfaceStateChangedSignal) {
        auto interfaceChangeDetails = check_and_cast<NetworkInterfaceChangeDetails *>(details);
        if (interfaceChangeDetails->getFieldId() == NetworkInterface::F_CARRIER && !networkInterface->hasCarrier())
            clearCollection();
    }
    else if (signal == packetPushedSignal) {
        if (!networkInterface->hasCarrier())
            clearCollection();
    }
    else
        throw cRuntimeError("Unknown signal");
}

} // namespace inet

