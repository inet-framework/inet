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
        packetCollection = getModuleFromPar<IPacketCollection>(par("collectionModule"), this);
        auto packetCollectionModule = check_and_cast<cModule *>(packetCollection);
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

