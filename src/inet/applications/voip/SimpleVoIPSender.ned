//
// Copyright (C) 2011 Adriano (University of Pisa)
// Copyright (C) 2012 Opensim Ltd.
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

package inet.applications.voip;

import inet.applications.IUDPApp;

//
// Implements a simple VoIP source. It is a constant bitrate source, with 
// talkspurt support added. Packets do not contain actual voice data.
// Connection setup/teardown is not modelled. The peer must be a 
// SimpleVoIPReceiver.
//
// The source alternates between two states: talk and silence. The length
// the module spends in each state are controlled by the talkspurtDuration and
// silenceDuration parameters. In talk state, the module acts as a CBR source,
// and sends packets of size talkPacketSize every packetizationInterval seconds
// to the destination over UDP. Silence is implicit: in silence state, no packets
// are sent, and there is no explicit signalling of silence either. 
//
// Packets are of type SimpleVoIPPacket. In addition to packet ID and timestamp,
// packets also contain a talkspurt ID and the total number of packets in the 
// talkspurt to facilitate statistics at the receiver side. (It allows the receiver
// to detect loss of packets at the end of a talkspurt; a full talkspurt can still
// be lost unnoticed.)
// 
// The destination address/port, the local port, start and stop times can be
// set via parameters.
//
// @authors: Adriano, Zoltan Bojthe
//
simple SimpleVoIPSender like IUDPApp
{
    parameters:
        int localPort = default(-1);
        int destPort;
        string destAddress;
        int talkPacketSize @unit(B) = default(40B);     // size of talk packets in bytes
        volatile double talkspurtDuration @unit(s) = default(weibull(1.423s, 0.824s));
        volatile double silenceDuration @unit(s) = default(weibull(0.899s, 1.089s));
        double packetizationInterval @unit(s) = default(20ms);   // interval between sending voice packets
        double startTime @unit(s) = default(0s);   // time of start sending
        double stopTime @unit(s) = default(-1s);  // time of end of sending, -1 means forever

        @display("i=block/source");
    gates:
        output udpOut;
        input udpIn;
}
