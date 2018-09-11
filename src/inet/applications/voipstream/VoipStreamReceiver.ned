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

import inet.applications.contract.IApp;

//
// VoipStreamReceiver listens on an UDP port, and expects to receive VoIP packets on
// it. The received voice is then saved into a result audio file that can be
// compared with the original for further evaluation. VoIP packets are
// numbered, and out-of-order packets are discarded (the corresponding voice
// interval will be recorded as silence into the file). VoIP packets that miss
// their deadlines will similarly be discarded. It is assumed that the audio is
// played back with delay (by default 20ms), which allows some jitter for the
// incoming packets. The resulting audio file is closed when the simulation
// completes (i.e. in the OMNeT++ finish() function). Only one voice session
// ("call") may be underway at a time.
//
simple VoipStreamReceiver like IApp
{
    parameters:
        int localPort;
        double playoutDelay @unit(s) = default(20ms);
        string resultFile;
        @signal[packetReceived](type=inet::Packet); // expected type=VoipStreamPacket
        @signal[packetDropped](type=inet::Packet);
        @signal[lostSamples](type=long);
        @signal[lostPackets](type=long);
        @signal[packetHasVoice](type=long);  // 1=yes, 0=no
        @signal[connState](type=long);  // 1=open, -1=close
        @signal[delay](type=simtime_t);  // total time lag of voice transmission
        @statistic[packetReceived](title="packets received"; source=packetReceived; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[dropPk](title="packets dropped"; source=packetDropped; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[lostSamples](title="lost samples"; interpolationmode=none; record=vector,stats);
        @statistic[lostPackets](title="lost packets"; interpolationmode=none; record=vector,stats);
        @statistic[packetHasVoice](title="voice or silence packet"; record=mean,vector; interpolationmode=none);
        @statistic[delay](title="delay"; unit=s; record=vector,histogram; interpolationmode=none);
        @statistic[numActiveSessions](title="number of active sessions"; source="sum(connState)"; record=max,timeavg,vector; interpolationmode=sample-hold; );
        @statistic[numSessions](title="total number of sessions"; source="sum(connState+1)/2"; record=last);
        @display("i=block/arrival");
    gates:
        input socketIn @labels(UdpControlInfo/up);
        output socketOut @labels(UdpControlInfo/down);
}

