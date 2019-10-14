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

#include <algorithm>
#include "inet/queueing/PacketBuffer.h"
#include "inet/common/Simsignals.h"
#include "inet/common/StringFormat.h"

namespace inet {
namespace queueing {

Define_Module(PacketBuffer);

void PacketBuffer::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        displayStringTextFormat = par("displayStringTextFormat");
        packetCapacity = par("packetCapacity");
        dataCapacity = b(par("dataCapacity"));
        const char *dropperClass = par("dropperClass");
        if (*dropperClass != '\0')
            packetDropperFunction = check_and_cast<IPacketDropperFunction *>(createOne(dropperClass));
    }
    else if (stage == INITSTAGE_LAST)
        updateDisplayString();
}

bool PacketBuffer::isOverloaded()
{
    return (packetCapacity != -1 && getNumPackets() > packetCapacity) ||
           (dataCapacity != b(-1) && getTotalLength() > dataCapacity);
}

void PacketBuffer::addPacket(Packet *packet)
{
    EV_INFO << "Adding packet " << packet->getName() << " to the buffer.\n";
    totalLength += packet->getTotalLength();
    packets.push_back(packet);
    if (isOverloaded())
        packetDropperFunction->dropPackets(this);
    updateDisplayString();
    emit(packetAddedSignal, packet);
}

void PacketBuffer::removePacket(Packet *packet)
{
    EV_INFO << "Removing packet " << packet->getName() << " from the buffer.\n";
    totalLength -= packet->getTotalLength();
    packets.erase(find(packets.begin(), packets.end(), packet));
    updateDisplayString();
    emit(packetRemovedSignal, packet);
    ICallback *callback = check_and_cast<ICallback *>(packet->getOwner()->getOwner());
    callback->handlePacketRemoved(packet);
}

void PacketBuffer::updateDisplayString()
{
    auto text = StringFormat::formatString(displayStringTextFormat, [&] (char directive) {
        static std::string result;
        switch (directive) {
            case 'p':
                result = std::to_string(packets.size());
                break;
            case 'l':
                result = totalLength.str();
                break;
            default:
                throw cRuntimeError("Unknown directive: %c", directive);
        }
        return result.c_str();
    });
    getDisplayString().setTagArg("t", 0, text);
}

Packet *PacketBuffer::getPacket(int index)
{
    if (index < 0 || (size_t)index >= packets.size())
        throw cRuntimeError("index %i out of range", index);
    return packets[index];
}

} // namespace queueing
} // namespace inet

