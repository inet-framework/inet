//
// This program is property of its copyright holder. All rights reserved.
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
