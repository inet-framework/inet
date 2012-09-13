//
// Copyright (C) 2005 M. Bohge (bohge@tkn.tu-berlin.de), M. Renwanz
// Copyright (C) 2010 Zoltan Bojthe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//


package inet.applications.voipstream;

import inet.applications.IUDPApp;


//
// VoIPStreamSender accepts an audio file and a destination IP address/port as
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
simple VoIPStreamSender like IUDPApp
{
    parameters:
        int localPort;
        int destPort;
        string destAddress;
        double startTime @unit(s) = default(0);
        int voipHeaderSize @unit(B);
        int voipSilenceThreshold;
        int sampleRate @unit(Hz) = default(8000Hz);
        string codec = default("g726");         // used by ffmpeg::avcodec_find_encoder_by_name()
        int compressedBitRate @unit(bps) = default(40kbps);
        double packetTimeLength @unit(s) = default(20ms);
        string soundFile;                       // file name of input audio file
        int repeatCount = default(1);
        string traceFileName = default("");     // file name to save output stream (wav), OFF when empty
        @signal[sentPk](type=VoIPPacket);
        @statistic[sentPk](title="packets sent"; source=sentPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @display("i=block/departure");
    gates:
        input udpIn;
        output udpOut;
}
