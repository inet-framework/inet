//
// (C) 2013 Opensim Ltd.
//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// Author: Andras Varga (andras@omnetpp.org)
//

#include "inet/linklayer/base/MacProtocolBase.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/IInterfaceRegistrationListener.h"

namespace inet {

MacProtocolBase::MacProtocolBase() :
    upperLayerInGateId(-1),
    upperLayerOutGateId(-1),
    lowerLayerInGateId(-1),
    lowerLayerOutGateId(-1),
    interfaceEntry(nullptr)
{
}

void MacProtocolBase::initialize(int stage)
{
    LayeredProtocolBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        upperLayerInGateId = findGate("upperLayerIn");
        upperLayerOutGateId = findGate("upperLayerOut");
        lowerLayerInGateId = findGate("lowerLayerIn");
        lowerLayerOutGateId = findGate("lowerLayerOut");
    }
}

void MacProtocolBase::registerInterface()
{
    ASSERT(interfaceEntry == nullptr);
    IInterfaceTable *interfaceTable = findModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
    if (interfaceTable) {
        interfaceEntry = createInterfaceEntry();
        interfaceTable->addInterface(interfaceEntry);
        auto module = getContainingNicModule(this);
        inet::registerInterface(*interfaceEntry, module->gate("upperLayerIn"), module->gate("upperLayerOut"));
    }
}

void MacProtocolBase::sendUp(cMessage *message)
{
    if (message->isPacket())
        emit(packetSentToUpperSignal, message);
    send(message, upperLayerOutGateId);
}

void MacProtocolBase::sendDown(cMessage *message)
{
    if (message->isPacket())
        emit(packetSentToLowerSignal, message);
    send(message, lowerLayerOutGateId);
}

bool MacProtocolBase::isUpperMessage(cMessage *message)
{
    return message->getArrivalGateId() == upperLayerInGateId;
}

bool MacProtocolBase::isLowerMessage(cMessage *message)
{
    return message->getArrivalGateId() == lowerLayerInGateId;
}

} // namespace inet

