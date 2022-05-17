//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ethernet/modular/EthernetMacLayer.h"

#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/queueing/contract/IPacketQueue.h"

namespace inet {

using namespace inet::queueing;

Define_Module(EthernetMacLayer);

void EthernetMacLayer::initialize(int stage)
{
    cModule::initialize(stage);
    if (stage == INITSTAGE_LINK_LAYER) {
        auto networkInterface = getContainingNicModule(this);
        auto interfaceTable = check_and_cast<cModule *>(networkInterface->getInterfaceTable());
        interfaceTable->subscribe(interfaceStateChangedSignal, this);
    }
}

void EthernetMacLayer::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signal));

    if (signal == interfaceStateChangedSignal) {
        auto networkInterface = getContainingNicModule(this);
        auto networkInterfaceChangeDetails = check_and_cast<const NetworkInterfaceChangeDetails *>(object);
        if (networkInterfaceChangeDetails->getNetworkInterface() == networkInterface && networkInterfaceChangeDetails->getFieldId() == NetworkInterface::F_CARRIER) {
            if (!networkInterface->hasCarrier()) {
                auto queue = check_and_cast<IPacketQueue *>(getSubmodule("queue"));
                if (queue != nullptr)
                    queue->removeAllPackets();
            }
        }
    }
    else
        throw cRuntimeError("Unknown signal");
}

} // namespace inet

