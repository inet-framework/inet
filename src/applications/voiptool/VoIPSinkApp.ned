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

package inet.applications.voiptool;

import inet.applications.IUDPApp;


//
// VoIPSinkApp listens on an UDP port, and expects to receive VoIP packets on
// it. The received voice is then saved into a result audio file that can be
// compared with the original for further evaluation. VoIP packets are
// numbered, and out-of-order packets are discarded (the corresponding voice
// interval will be recorded as silence into the file). VoIP packets that miss
// their deadlines will similarly be discarded. It is assumed that the audio is
// played back with delay (by default 20ms), which allows some jitter for the
// incoming packets. The resulting audio file is closed when the simulation
// completes (i.e. in the OMNeT++ finish() function).
//
simple VoIPSinkApp like IUDPApp
{
    parameters:
        int localPort;
        double delay @unit(s) = default(20ms);
        string resultFile;
        @statistic[receivedBytes](title="received bytes"; unit=B; record=vector,sum);
        @statistic[lostSamples](title="lost samples"; interpolationmode=none; record=vector,stats);
        @statistic[lostPackets](title="lost packets"; interpolationmode=none; record=vector,stats);
        @statistic[droppedBytes](title="dropped bytes"; interpolationmode=none; unit=B; record=vector,stats);
        @statistic[packetHasVoice](title="Voice/Silent"; record=vector,stats);
        @statistic[connState](title="Connection state"; interpolationmode=sample-hold; record=vector,stats);
        @statistic[delay](title="delay"; unit=s; record=vector,stats);
        @display("i=block/arrival");
    gates:
        input udpIn;
        output udpOut;
}
