#include <iostream>
#include <sstream>

#include "inet/transportlayer/rtp/RtcpPacket_m.h"

namespace inet {

namespace rtp {

void RtcpReceiverReportPacket::addReceptionReport(ReceptionReport *report)
{
    receptionReports.add(report);
    count++;
    // an rtcp receiver report is 24 bytes long
    setChunkLength(getChunkLength() + B(24));
};

void RtcpSdesPacket::addSDESChunk(SdesChunk *sdesChunk)
{
    sdesChunks.add(sdesChunk);
    count++;
    // the size of the rtcp packet increases by the
    // size of the sdes chunk (including ssrc)
    setChunkLength(getChunkLength() + B(sdesChunk->getLength()));
};

} // namespace rtp

} // namespace inet

