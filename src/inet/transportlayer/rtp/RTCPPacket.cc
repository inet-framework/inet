#include <iostream>
#include <sstream>

#include "inet/transportlayer/rtp/RTCPPacket.h"

namespace inet {

namespace rtp {

Register_Class(RtcpPacket);

Register_Class(RtcpReceiverReportPacket);

RtcpReceiverReportPacket::RtcpReceiverReportPacket()
    : RtcpReceiverReportPacket_Base()
{
    receptionReports.setName("ReceptionReports");
    // an empty rtcp receiver report packet is 4 bytes
    // longer, the ssrc identifier is stored in it
    setChunkLength(getChunkLength() + B(4));
};

void RtcpReceiverReportPacket::addReceptionReport(ReceptionReport *report)
{
    receptionReports.add(report);
    count++;
    // an rtcp receiver report is 24 bytes long
    setChunkLength(getChunkLength() + B(24));
};

Register_Class(RtcpSenderReportPacket);

RtcpSenderReportPacket::RtcpSenderReportPacket()
    : RtcpSenderReportPacket_Base()
{
    // a sender report is 20 bytes long
    setChunkLength(getChunkLength() + B(20));
};

Register_Class(RtcpSdesPacket);

RtcpSdesPacket::RtcpSdesPacket()
    : RtcpSdesPacket_Base()
{
    sdesChunks.setName("SDESChunks");
    // no addByteLength() needed, sdes chunks
    // directly follow the standard rtcp
    // header
};

void RtcpSdesPacket::addSDESChunk(SdesChunk *sdesChunk)
{
    sdesChunks.add(sdesChunk);
    count++;
    // the size of the rtcp packet increases by the
    // size of the sdes chunk (including ssrc)
    setChunkLength(getChunkLength() + B(sdesChunk->getLength()));
};

Register_Class(RtcpByePacket);

RtcpByePacket::RtcpByePacket()
    : RtcpByePacket_Base()
{
    // space for the ssrc identifier
    setChunkLength(getChunkLength() + B(4));
};

} // namespace rtp

} // namespace inet

