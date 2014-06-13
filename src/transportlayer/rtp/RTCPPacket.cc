#include <iostream>
#include <sstream>

#include "RTCPPacket.h"


Register_Class(RTCPPacket);


Register_Class(RTCPCompoundPacket);

void RTCPCompoundPacket::addRTCPPacket(RTCPPacket *rtcpPacket)
{
    //rtcpPacket->setOwner(_rtcpPackets);
    rtcpPackets_var.add(rtcpPacket);
    // the size of the rtcp compound packet increases
    // by the size of the added rtcp packet
    addByteLength(rtcpPacket->getByteLength());
};


Register_Class(RTCPReceiverReportPacket);

RTCPReceiverReportPacket::RTCPReceiverReportPacket(const char *name, int kind)
  : RTCPReceiverReportPacket_Base(name, kind)
{
    receptionReports_var.setName("ReceptionReports");
    // an empty rtcp receiver report packet is 4 bytes
    // longer, the ssrc identifier is stored in it
    addByteLength(4);
};

void RTCPReceiverReportPacket::addReceptionReport(ReceptionReport *report)
{
    receptionReports_var.add(report);
    count_var++;
    // an rtcp receiver report is 24 bytes long
    addByteLength(24);
};


Register_Class(RTCPSenderReportPacket);

RTCPSenderReportPacket::RTCPSenderReportPacket(const char *name, int kind)
  : RTCPSenderReportPacket_Base(name, kind)
{
    // a sender report is 20 bytes long
    addByteLength(20);
};


Register_Class(RTCPSDESPacket);

RTCPSDESPacket::RTCPSDESPacket(const char *name, int kind)
  : RTCPSDESPacket_Base(name, kind)
{
    sdesChunks_var.setName("SDESChunks");
    // no addByteLength() needed, sdes chunks
    // directly follow the standard rtcp
    // header
};

void RTCPSDESPacket::addSDESChunk(SDESChunk *sdesChunk)
{
    sdesChunks_var.add(sdesChunk);
    count_var++;
    // the size of the rtcp packet increases by the
    // size of the sdes chunk (including ssrc)
    addByteLength(sdesChunk->getLength());
};


Register_Class(RTCPByePacket);

RTCPByePacket::RTCPByePacket(const char *name, int kind)
  : RTCPByePacket_Base(name, kind)
{
    // space for the ssrc identifier
    addByteLength(4);
};
