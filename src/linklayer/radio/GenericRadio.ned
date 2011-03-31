//
// Copyright (C) Andras Varga
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


package inet.linklayer.radio;

//
// Generic radio module. The implementation is largely based on the
// Mobility Framework's ~SnrEval and Decider modules.
//
// @see ~IRadio
// @author Andras Varga
//
simple GenericRadio extends PropagationModel
{
    parameters:
        int channelNumber = default(0); // channel identifier
        double transmitterPower @unit("mW") = default(20mW); // power used for transmission of messages (in mW)
        double bitrate @unit("bps"); // (in bits/s)
        double thermalNoise @unit("dBm") = default(-110dBm); // base noise level (dBm)
        double pathLossAlpha = default(2); // used by the path loss calculation
        double snirThreshold @unit("dB") = default(4dB); // if signal-noise ratio is below this threshold, frame is considered noise (in dB)
        double sensitivity @unit("dBm") = default(-85dBm); // received signals with power below sensitivity are ignored
        int headerLengthBits @unit(b); // length of physical layer framing (preamble, etc)
        double bandwidth @unit("Hz"); // signal bandwidth, used for bit error calculation
        string modulation; // "BPSK", "16-QAM", "256-QAM" or "null"; selects bit error calculation method
        string attenuationModel = default("PathLossReceptionModel");
        bool drawCoverage =default(true);
        
        double usage_radio_idle @unit(mA) = default (-1mA); // disable battery registration
        double usage_radio_recv @unit(mA) = default (-1mA); // disable battery registration
        double usage_radio_sleep @unit(mA) = default (-1mA); // disable battery registration 
        double usage_radio_send @unit(mA) = default (-1mA); // disable battery registration 

        @display("i=block/wrxtx");
        @statistic[bitrate](title="Bitrate"; record=timeavg,vector; interpolationmode=sample-hold; );
        @statistic[channelNumber](title="Channel no"; record=histogram,vector; interpolationmode=sample-hold; ); //vector
        @statistic[radioState](title="Radio state (enum)"; record=histogram,vector; interpolationmode=sample-hold; ); //vector
        @class(GenericRadio);
}
