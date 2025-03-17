//
// Copyright (C) 2005 M. Bohge (bohge@tkn.tu-berlin.de), M. Renwanz
// Copyright (C) 2010 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.applications.voipstream;

import inet.applications.contract.IApp;

//
// Implements a realistic VoIP receiver that processes incoming audio packets and
// reconstructs the original audio stream with consideration for network impairments.
// Listens on a specified UDP port for packets from a VoipStreamSender, decodes the
// audio data, and saves the reconstructed stream to a file for quality assessment
// and comparison with the original audio.
//
// Incorporates sophisticated packet handling mechanisms that model real VoIP systems,
// including playout buffering with configurable delay to absorb network jitter,
// packet sequence checking to handle out-of-order delivery, and deadline enforcement
// to discard late packets. Maintains detailed statistics on various quality metrics
// including packet loss, silence periods, and end-to-end delay, providing comprehensive
// data for analyzing VoIP performance under different network conditions.
//
// @see ~VoipStreamSender, ~UdpSocket
//
simple VoipStreamReceiver like IApp
{
    parameters:
        int localPort;
        double playoutDelay @unit(s) = default(20ms);
        string resultFile;
        @signal[packetReceived](type=inet::Packet); // Expected type=VoipStreamPacket
        @signal[packetDropped](type=inet::Packet);
        @signal[lostSamples](type=long);
        @signal[lostPackets](type=long);
        @signal[packetHasVoice](type=long);  // 1=yes, 0=no
        @signal[connState](type=long);  // 1=open, -1=close
        @signal[delay](type=simtime_t);  // Total time lag of voice transmission
        @statistic[packetReceived](title="packets received"; source=packetReceived; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[dropPk](title="packets dropped"; source=packetDropped; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[lostSamples](title="lost samples"; interpolationmode=none; record=vector,stats);
        @statistic[lostPackets](title="lost packets"; interpolationmode=none; record=vector,stats);
        @statistic[packetHasVoice](title="voice or silence packet"; record=mean,vector; interpolationmode=none);
        @statistic[delay](title="delay"; unit=s; record=vector,histogram; interpolationmode=none);
        @statistic[numActiveSessions](title="number of active sessions"; source=warmup(sum(connState)); record=max,timeavg,vector; interpolationmode=sample-hold; autoWarmupFilter=false);
        @statistic[numSessions](title="total number of sessions"; source="sum(connState+1)/2"; record=last);
        @display("i=block/arrival");
    gates:
        input socketIn @labels(UdpCommand/up);
        output socketOut @labels(UdpCommand/down);
}
