//
// Copyright (C) 2005 M. Bohge (bohge@tkn.tu-berlin.de), M. Renwanz
// Copyright (C) 2010 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.applications.voipstream;

import inet.applications.contract.IApp;

//
// VoipStreamSender accepts an audio file and a destination IP address/port as
// input, and will transmit the file's contents as voice traffic over UDP n
// times (by default once). For transmission, the audio is resampled at the
// specified frequency and depth, and encoded with the specified codec at the
// specified bit rate, and chopped into packets that each carry specified
// number of milliseconds of voice. Those values come from module parameters.
// Packets that are all silence (all samples are below a given threshold in
// absolute value) are transmitted as special "silence" packets. The module
// does not simulate any particular VoIP protocol (e.g. RTP), but instead
// accepts a "header size" parameter that can be set accordingly.
//
simple VoipStreamSender like IApp
{
    parameters:
        int localPort;
        int destPort;
        string destAddress;
        double startTime @unit(s) = default(0s);
        int voipHeaderSize @unit(B);
        int voipSilenceThreshold;
        int sampleRate @unit(Hz) = default(8000Hz);
        string codec = default("g726");         // used by ffmpeg::avcodec_find_encoder_by_name()
        int compressedBitRate @unit(bps) = default(40kbps);
        double packetTimeLength @unit(s) = default(20ms);
        string soundFile;                       // file name of input audio file
        int repeatCount = default(1);
        string traceFileName = default("");     // file name to save output stream (wav), OFF when empty
        int timeToLive = default(-1); // if not -1, set the TTL (IPv4) or Hop Limit (IPv6) field of sent packets to this value
        int dscp = default(-1); // if not -1, set the DSCP field (on IPv4/IPv6) of sent packets to this value
        int tos = default(-1); // if not -1, set the Type Of Service (IPv4) / Traffic Class (IPv6) field of sent packets to this value
        @signal[packetSent](type=inet::Packet);       // TODO first chunk is VoipStreamPacket
        @statistic[packetSent](title="packets sent"; source=packetSent; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @display("i=block/departure");
    gates:
        input socketIn @labels(UdpControlInfo/up);
        output socketOut @labels(UdpControlInfo/down);
}

