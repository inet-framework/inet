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

import inet.common.INETDefs;
import inet.common.TagBase;
import inet.common.Units;

namespace inet;

enum PacketEventKind
{
    PEK_UNDEFINED    = -1;
    PEK_DELAYED      = 0;
    PEK_QUEUED       = 1;
    PEK_PROCESSED    = 2;
    PEK_TRANSMITTED  = 3;
    PEK_PROPAGATED   = 4;
}

class PacketEvent extends cObject
{
    @str(getClassName());
    int kind @enum(PacketEventKind) = PEK_UNDEFINED;
    string modulePath;
    int64_t eventNumber = -1;
    simtime_t simulationTime = -1;
    simtime_t duration = -1;
    b packetLength = b(-1);
}

class PacketQueuedEvent extends PacketEvent
{
    int queuePacketLength = -1;
    b queueDataLength = b(-1);
}

class PacketTransmittedEvent extends PacketEvent
{
    bps datarate = bps(-1);
}

class PacketEventTag extends TagBase
{
    PacketEvent *packetEvents[];
}
