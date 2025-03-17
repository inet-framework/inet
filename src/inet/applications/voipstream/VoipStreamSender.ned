//
// Copyright (C) 2005 M. Bohge (bohge@tkn.tu-berlin.de), M. Renwanz
// Copyright (C) 2010 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.applications.voipstream;

import inet.applications.contract.IApp;

//
// Implements a realistic VoIP traffic source that processes actual audio files to
// generate network traffic with authentic voice characteristics. Reads audio data
// from a specified file, processes it through a complete audio pipeline including
// resampling, encoding with industry-standard codecs (using FFmpeg libraries), and
// packetization, then transmits the resulting stream over UDP to a specified destination.
//
// Provides extensive configuration options for audio processing parameters including
// sample rate, codec selection, compression bit rate, and packet timing. Implements
// silence detection to identify and specially mark silent periods in the audio stream,
// optimizing bandwidth usage. The module can be configured to repeat the audio file
// multiple times, allowing for extended simulation scenarios with consistent audio
// patterns. While not implementing a specific VoIP protocol like RTP, it includes
// configurable header size to accurately model protocol overhead.
//
// @see ~VoipStreamReceiver, ~UdpSocket
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
        string codec = default("g726");         // Used by ffmpeg::avcodec_find_encoder_by_name()
        int compressedBitRate @unit(bps) = default(40kbps);
        double packetTimeLength @unit(s) = default(20ms);
        string soundFile;                       // File name of input audio file
        int repeatCount = default(1);
        string traceFileName = default("");     // File name to save output stream (wav), OFF when empty
        int timeToLive = default(-1); // If not -1, set the TTL (IPv4) or Hop Limit (IPv6) field of sent packets to this value
        int dscp = default(-1); // If not -1, set the DSCP field (on IPv4/IPv6) of sent packets to this value
        int tos = default(-1); // If not -1, set the Type Of Service (IPv4) / Traffic Class (IPv6) field of sent packets to this value
        @signal[packetSent](type=inet::Packet);       // TODO first chunk is VoipStreamPacket
        @statistic[packetSent](title="packets sent"; source=packetSent; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @display("i=block/departure");
    gates:
        input socketIn @labels(UdpCommand/up);
        output socketOut @labels(UdpCommand/down);
}
