//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// at your option) any later version.
//
// (C) 2001 by Matthias Oppitz <Matthias.Oppitz@gmx.de>
//

import inet.common.INETDefs;

namespace inet::rtp;

//
// Represents an RTP sender report as contained
// in an ~RtcpSenderReportPacket.
//
class SenderReport extends cObject
{
    // The ntp time stamp.
    uint64 NTPTimeStamp;

    // The rtp time stamp.
    uint32 RTPTimeStamp;

    // The number of packets sent.
    uint32 packetCount;

    // The number of (payload) bytes sent.
    uint32 byteCount;
};

cplusplus(SenderReport) {{
  public:
    /**
     * Writes a short info about this SenderReport into the given string.
     */
    virtual std::string str() const override;

    /**
     * Writes a longer info about this SenderReport into the given stream.
     */
    virtual void dump(std::ostream& os) const;
}}


//
// Rrepresents an RTP receiver report stored
// in an RTPSenderReportPacket or RTPReceiverReport.
//
class ReceptionReport extends cObject
{
    // The ssrc identifier of the sender this ~ReceptionReport is for.
    uint32 ssrc;

    // The fraction lost.
    uint8 fractionLost;

    // The number of packets expected minus the number of packets received.
    int packetsLostCumulative;

    // The extended highest sequence number received.
    uint32 sequenceNumber;

    // The interarrival jitter.
    int jitter;

    // The rtp time stamp of the last ~SenderReport received from this source.
    int lastSR;

    // The delay since the last ~SenderReport from this sender has been
    // received in units of 1/65536 seconds.
    int delaySinceLastSR;
};

cplusplus(ReceptionReport) {{
  public:
    /**
     * Writes a short info about this ReceptionReport into the given string.
     */
    virtual std::string str() const override;

    /**
     * Writes a longer info about this ReceptionReport into the given stream.
     */
    virtual void dump(std::ostream& os) const;
}}

