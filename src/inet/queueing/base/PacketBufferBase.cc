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

#include "inet/queueing/base/PacketBufferBase.h"
#include "inet/common/Simsignals.h"
#include "inet/common/StringFormat.h"

namespace inet {
namespace queueing {

void PacketBufferBase::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        displayStringTextFormat = par("displayStringTextFormat");
        numAddedPackets = 0;
        numRemovedPackets = 0;
        numDroppedPackets = 0;
        WATCH(numAddedPackets);
        WATCH(numRemovedPackets);
        WATCH(numDroppedPackets);
    }
}

void PacketBufferBase::emit(simsignal_t signal, cObject *object, cObject *details)
{
    if (signal == packetAddedSignal)
        numAddedPackets++;
    else if (signal == packetRemovedSignal)
        numRemovedPackets++;
    else if (signal == packetDroppedSignal)
        numDroppedPackets++;
    cSimpleModule::emit(signal, object, details);
}

void PacketBufferBase::updateDisplayString()
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
                case 'a':
                    result = std::to_string(numAddedPackets);
                    break;
                case 'r':
                    result = std::to_string(numRemovedPackets);
                    break;
                case 'd':
                    result = std::to_string(numDroppedPackets);
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

