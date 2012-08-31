
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
simple VoIPReceiver like IUDPApp
{
    parameters:
        int localPort;
        int emodel_Ie_ = default(5);
        int emodel_Bpl_ = default(10);
        int emodel_A_ = default(5);
        double emodel_Ro_ = default(93.2);
        double playoutDelay @unit(s) = default(0);      // delay for begin playout after received first packet
        int dim_buffer = default(20);
        double samplingTime @unit(s) = default(20ms);

        @signal[VoIPFrameDelay](type="simtime_t");      // when receive a frame
        @signal[VoIPFrameLossRate](type="double");      // at end of talkspur
        @signal[VoIPPlayoutDelay](type="simtime_t");    // at end of talkspur
        @signal[VoIPPlayoutLossRate](type="double");    // at end of talkspur
        @signal[VoIPMosSignal](type="double");          // at end of talkspur
        @signal[VoIPTaildropLossRate](type="double");   // at end of talkspur
        @statistic[VoIPFrameDelay](title="VoIP Frame Delay"; unit="s"; source="VoIPFrameDelay"; record=stats,vector); 
        @statistic[VoIPFrameLossRate](title="VoIP Frame Loss"; unit="ratio"; source="VoIPFrameLossRate"; record=stats,vector);
        @statistic[VoIPPlayoutDelay](title="VoIP Playout Delay"; unit="s"; source="VoIPPlayoutDelay"; record=stats,vector);
        @statistic[VoIPPlayoutLossRate](title="VoIP Playout Loss"; unit="ratio"; source="VoIPPlayoutLossRate"; record=stats,vector);
        @statistic[VoIPMosSignal](title="VoIP Mos Signal"; unit="MOS"; source="VoIPMosSignal"; record=stats,vector);
        @statistic[VoIPTaildropLossRate](title="VoIP Tail Drop Loss"; unit="ratio"; source="VoIPTaildropLossRate"; record=stats,vector);

        @display("i=block/source");
    gates:
        output udpOut;
        input udpIn;
}

