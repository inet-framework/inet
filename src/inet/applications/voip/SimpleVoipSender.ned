//
// Copyright (C) 2011 Adriano (University of Pisa)
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.applications.voip;

import inet.applications.contract.IApp;

//
// Implements a simple VoIP source. It is a constant bitrate source, with
// talk spurt support added. Packets do not contain actual voice data.
// Connection setup/teardown is not modeled. The peer must be a
// ~SimpleVoipReceiver.
//
// The source alternates between two states: talk and silence. The length
// the module spends in each state is controlled by the `talkspurtDuration` and
// `silenceDuration` parameters. In the talk state, the module acts as a CBR source,
// and sends packets of size `talkPacketSize` every `packetizationInterval` seconds
// to the destination over UDP. Silence is implicit: in the silence state, no packets
// are sent, and there is no explicit signaling of silence either.
//
// Packets are of type `SimpleVoipPacket`. In addition to packet ID and timestamp,
// packets also contain a talkspurt ID and the total number of packets in the
// talkspurt to facilitate statistics on the receiver's side. (It allows the receiver
// to detect the loss of packets at the end of a talkspurt; a full talkspurt can still
// be lost unnoticed.)
//
// The destination address/port, the local port, start and stop times can be
// set via parameters.
//
// @author: Adriano
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
        input socketIn @labels(UdpControlInfo/up);
        output socketOut @labels(UdpControlInfo/down);
}

