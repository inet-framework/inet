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
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "inet/common/ModuleAccess.h"
#include "inet/protocol/transceiver/base/PacketReceiverBase.h"

namespace inet {

void PacketReceiverBase::initialize(int stage)
{
    PacketProcessorBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        inputGate = gate("in");
        outputGate = gate("out");
        consumer = findConnectedModule<IPassivePacketSink>(outputGate);
    }
    else if (stage == INITSTAGE_QUEUEING)
        checkPacketOperationSupport(outputGate);
}

Packet *PacketReceiverBase::decodePacket(Signal *signal) const
{
    auto packet = check_and_cast<Packet *>(signal->decapsulate());
    // TODO: check signal physical properties such as datarate, modulation, etc.
    return packet;
}

} // namespace inet
