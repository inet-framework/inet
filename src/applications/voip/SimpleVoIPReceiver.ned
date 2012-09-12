
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 
package inet.applications.voip;

import inet.applications.IUDPApp;


//
//FIXME Documentation!!!
//
simple SimpleVoIPReceiver like IUDPApp
{
    parameters:
        int localPort;

        // FIXME: parameter names should not end with _, in fact I believe they should not even contain any _ character
        // e-model defined in ITU-T G.107 standard
        int emodel_Ie_ = default(5);        // Equipment impairment factor
        int emodel_Bpl_ = default(10);      // Packet-loss robustness factor
        int emodel_A_ = default(5);         // Advantage factor
        double emodel_Ro_ = default(93.2);  // Basic signal-to-noise ratio

        double playoutDelay @unit(s) = default(0);      // delay for begin playout after received first packet
        int bufferSpace = default(20);      // buffer size (max number of packets in buffer)

        @signal[VoIPPacketDelay](type="simtime_t");      // when receive a frame
        @signal[VoIPPacketLossRate](type="double");      // at end of talkspur
        @signal[VoIPPlayoutDelay](type="simtime_t");    // at end of talkspur
        @signal[VoIPPlayoutLossRate](type="double");    // at end of talkspur
        @signal[VoIPMosSignal](type="double");          // at end of talkspur
        @signal[VoIPTaildropLossRate](type="double");   // at end of talkspur
        @statistic[VoIPPacketDelay](title="VoIP Packet Delay"; unit="s"; source="VoIPPacketDelay"; record=stats,vector); 
        @statistic[VoIPPacketLossRate](title="VoIP Packet Loss"; unit="ratio"; source="VoIPPacketLossRate"; record=stats,vector);
        @statistic[VoIPPlayoutDelay](title="VoIP Playout Delay"; unit="s"; source="VoIPPlayoutDelay"; record=stats,vector);
        @statistic[VoIPPlayoutLossRate](title="VoIP Playout Loss"; unit="ratio"; source="VoIPPlayoutLossRate"; record=stats,vector);
        @statistic[VoIPMosSignal](title="VoIP Mos Signal"; unit="MOS"; source="VoIPMosSignal"; record=stats,vector);
        @statistic[VoIPTaildropLossRate](title="VoIP Tail Drop Loss"; unit="ratio"; source="VoIPTaildropLossRate"; record=stats,vector);

        @display("i=block/source");
    gates:
        output udpOut;
        input udpIn;
}

