//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/flow/PacketFlowPcapFileRecorder.h"

#include "inet/common/packet/recorder/PcapWriter.h"
#include "inet/common/packet/recorder/PcapngWriter.h"

namespace inet {
namespace queueing {

Define_Module(PacketFlowPcapFileRecorder);

void PacketFlowPcapFileRecorder::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        const char *fileFormat = par("fileFormat");
        if (!strcmp(fileFormat, "pcap"))
            pcapWriter = new PcapWriter();
        else if (!strcmp(fileFormat, "pcapng"))
            pcapWriter = new PcapngWriter();
        else
            throw cRuntimeError("Unknown fileFormat parameter");
        pcapWriter->setFlush(par("alwaysFlush"));
        pcapWriter->open(par("filename"), par("snaplen"), par("timePrecision"));
        networkType = static_cast<PcapLinkType>(par("networkType").intValue());
        const char *dirString = par("direction");
        if (*dirString == 0)
            direction = DIRECTION_UNDEFINED;
        else if (!strcmp(dirString, "outbound"))
            direction = DIRECTION_OUTBOUND;
        else if (!strcmp(dirString, "inbound"))
            direction = DIRECTION_INBOUND;
        else
            throw cRuntimeError("invalid direction parameter value: %s", dirString);
    }
}

void PacketFlowPcapFileRecorder::finish()
{
    pcapWriter->close();
}

void PacketFlowPcapFileRecorder::processPacket(Packet *packet)
{
    pcapWriter->writePacket(simTime(), packet, direction, findContainingNicModule(this), networkType);
}

} // namespace queueing
} // namespace inet

