//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#include "inet/common/ModuleAccess.h"
#include "inet/queueing/base/PacketServerBase.h"

namespace inet {
namespace queueing {

simsignal_t PacketServerBase::packetServedSignal = cComponent::registerSignal("packetServed");

void PacketServerBase::initialize(int stage)
{
    PacketSinkBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        inputGate = gate("in");
        provider = getConnectedModule<IPassivePacketSource>(inputGate);
        outputGate = gate("out");
        consumer = getConnectedModule<IPassivePacketSink>(outputGate);
    }
    else if (stage == INITSTAGE_QUEUEING) {
        checkPopPacketSupport(inputGate);
        checkPushPacketSupport(outputGate);
    }
}

} // namespace queueing
} // namespace inet

