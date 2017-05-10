#include <iostream>
#include <sstream>

#include "inet/transportlayer/rtp/RTCPPacket.h"

namespace inet {

namespace rtp {

Register_Class(RTCPPacket);

Register_Class(RTCPReceiverReportPacket);

RTCPReceiverReportPacket::RTCPReceiverReportPacket()
    : RTCPReceiverReportPacket_Base()
{
    receptionReports.setName("ReceptionReports");
    // an empty rtcp receiver report packet is 4 bytes
    // longer, the ssrc identifier is stored in it
    setChunkLength(getChunkLength() + byte(4));
};

void RTCPReceiverReportPacket::addReceptionReport(ReceptionReport *report)
{
    receptionReports.add(report);
    count++;
    // an rtcp receiver report is 24 bytes long
    setChunkLength(getChunkLength() + byte(24));
};

Register_Class(RTCPSenderReportPacket);

RTCPSenderReportPacket::RTCPSenderReportPacket()
    : RTCPSenderReportPacket_Base()
{
    // a sender report is 20 bytes long
    setChunkLength(getChunkLength() + byte(20));
};

Register_Class(RTCPSDESPacket);

RTCPSDESPacket::RTCPSDESPacket()
    : RTCPSDESPacket_Base()
{
    sdesChunks.setName("SDESChunks");
    // no addByteLength() needed, sdes chunks
    // directly follow the standard rtcp
    // header
};

void RTCPSDESPacket::addSDESChunk(SDESChunk *sdesChunk)
{
    sdesChunks.add(sdesChunk);
    count++;
    // the size of the rtcp packet increases by the
    // size of the sdes chunk (including ssrc)
    setChunkLength(getChunkLength() + byte(sdesChunk->getLength()));
};

Register_Class(RTCPByePacket);

RTCPByePacket::RTCPByePacket()
    : RTCPByePacket_Base()
{
    // space for the ssrc identifier
    setChunkLength(getChunkLength() + byte(4));
};

} // namespace rtp

} // namespace inet

