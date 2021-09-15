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

