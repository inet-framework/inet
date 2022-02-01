//
// Copyright (C) 2001 Matthias Oppitz <Matthias.Oppitz@gmx.de>
// Copyright (C) 2017 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include <iostream>
#include <sstream>

#include "inet/transportlayer/rtp/RtcpPacket_m.h"

namespace inet {

namespace rtp {

void RtcpPacket::paddingAndSetLength()
{
    handleChange();
    int64_t chunkBits = getChunkLength().get();
    rtcpLength = (chunkBits + 31) / 32 - 1;
    setChunkLength(b((rtcpLength + 1) * 32));
}

void RtcpReceiverReportPacket::addReceptionReport(ReceptionReport *report)
{
    handleChange();
    receptionReports.add(report);
    count++;
    // an rtcp receiver report is 24 bytes long
    addChunkLength(B(24));
};

void RtcpSdesPacket::addSDESChunk(SdesChunk *sdesChunk)
{
    handleChange();
    sdesChunks.add(sdesChunk);
    count++;
    // the size of the rtcp packet increases by the
    // size of the sdes chunk (including ssrc)
    addChunkLength(B(sdesChunk->getLength()));
};

} // namespace rtp

} // namespace inet

