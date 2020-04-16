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

#include "inet/queueing/base/PacketQueueBase.h"
#include "inet/common/Simsignals.h"
#include "inet/common/StringFormat.h"

namespace inet {
namespace queueing {

void PacketQueueBase::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        inputGate = gate("in");
        outputGate = gate("out");
        displayStringTextFormat = par("displayStringTextFormat");
        numPushedPackets = 0;
        numPulledPackets = 0;
        numRemovedPackets = 0;
        numDroppedPackets = 0;
        numCreatedPackets = 0;
        WATCH(numPushedPackets);
        WATCH(numPulledPackets);
        WATCH(numRemovedPackets);
        WATCH(numDroppedPackets);
    }
}

void PacketQueueBase::handleMessage(cMessage *message)
{
    auto packet = check_and_cast<Packet *>(message);
    pushPacket(packet, packet->getArrivalGate());
}

void PacketQueueBase::emit(simsignal_t signal, cObject *object, cObject *details)
{
    if (signal == packetPushedSignal)
        numPushedPackets++;
    else if (signal == packetPulledSignal)
        numPulledPackets++;
    else if (signal == packetRemovedSignal)
        numRemovedPackets++;
    else if (signal == packetDroppedSignal)
        numDroppedPackets++;
    cSimpleModule::emit(signal, object, details);
}

void PacketQueueBase::updateDisplayString()
{
    if (getEnvir()->isGUI()) {
        auto text = StringFormat::formatString(displayStringTextFormat, [&] (char directive) {
            static std::string result;
            switch (directive) {
                case 'p':
                    result = std::to_string(getNumPackets());
                    break;
                case 'l':
                    result = getTotalLength().str();
                    break;
                case 'u':
                    result = std::to_string(numPushedPackets);
                    break;
                case 'o':
                    result = std::to_string(numPulledPackets);
                    break;
                case 'r':
                    result = std::to_string(numRemovedPackets);
                    break;
                case 'd':
                    result = std::to_string(numDroppedPackets);
                    break;
                case 'c':
                    result = std::to_string(numCreatedPackets);
                    break;
                case 'n':
                    result = !isEmpty() ? getPacket(0)->getFullName() : "";
                    break;
                default:
                    throw cRuntimeError("Unknown directive: %c", directive);
            }
            return result.c_str();
        });
        getDisplayString().setTagArg("t", 0, text);
    }
}

} // namespace queueing
} // namespace inet

