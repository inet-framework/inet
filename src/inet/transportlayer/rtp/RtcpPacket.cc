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
    setChunkLength(b((rtcpLength+1) * 32));
}

void RtcpReceiverReportPacket::addReceptionReport(ReceptionReport *report)
{
    handleChange();
    receptionReports.add(report);
    count++;
    // an rtcp receiver report is 24 bytes long
    setChunkLength(getChunkLength() + B(24));
};

void RtcpSdesPacket::addSDESChunk(SdesChunk *sdesChunk)
{
    handleChange();
    sdesChunks.add(sdesChunk);
    count++;
    // the size of the rtcp packet increases by the
    // size of the sdes chunk (including ssrc)
    setChunkLength(getChunkLength() + B(sdesChunk->getLength()));
};

} // namespace rtp

} // namespace inet

