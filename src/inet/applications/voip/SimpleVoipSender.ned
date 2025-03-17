//
// Copyright (C) 2011 Adriano (University of Pisa)
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.applications.voip;

import inet.applications.contract.IApp;

//
// Simulates a VoIP traffic source that generates packet streams with realistic
// speech patterns, alternating between talk spurts and silence periods. During talk
// spurts, packets of configurable size are sent at regular intervals, while no packets
// are transmitted during silence periods. The durations of both talk and silence phases
// can be configured with statistical distributions (default: Weibull) to model natural
// conversation patterns.
//
// Each packet contains metadata including timestamps, talkspurt identification, and
// sequence information, allowing the receiver to calculate quality metrics such as
// packet loss, delay, and jitter. The implementation focuses on traffic pattern
// simulation rather than actual voice encoding, making it suitable for network
// performance studies without the computational overhead of real audio processing.
//
// Designed to work with ~SimpleVoipReceiver, which calculates quality metrics including
// Mean Opinion Score (MOS) based on the received packet stream characteristics.
//
// @see ~SimpleVoipReceiver, ~SimpleVoipPacket, ~UdpSocket
//
simple SimpleVoipSender like IApp
{
    parameters:
        int localPort = default(-1);
        int destPort;
        string destAddress;
        int talkPacketSize @unit(B) = default(40B);     // Size of talk packets in bytes
        volatile double talkspurtDuration @unit(s) = default(weibull(1.423s, 0.824s));
        volatile double silenceDuration @unit(s) = default(weibull(0.899s, 1.089s));
        double packetizationInterval @unit(s) = default(20ms);   // Interval between sending voice packets
        double startTime @unit(s) = default(0s);   // Time of start sending
        double stopTime @unit(s) = default(-1s);  // Time of end of sending, -1 means forever

        @display("i=block/source");
    gates:
        input socketIn @labels(UdpCommand/up);
        output socketOut @labels(UdpCommand/down);
}
