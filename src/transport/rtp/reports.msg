//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// at your option) any later version.
//
// (C) 2001 by Matthias Oppitz <Matthias.Oppitz@gmx.de>
//

//
// Represents an RTP sender report as contained
// in an ~RTCPSenderReportPacket.
//
class SenderReport
{
    @customize(true);

    // The ntp time stamp.
    uint64 NTPTimeStamp;

    // The rtp time stamp.
    uint32 RTPTimeStamp;

    // The number of packets sent.
    uint32 packetCount;

    // The number of (payload) bytes sent.
    uint32 byteCount;
};

//
// Rrepresents an RTP receiver report stored
// in an RTPSenderReportPacket or RTPReceiverReport.
//
class ReceptionReport
{
    @customize(true);

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
