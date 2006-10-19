//
// Copyright (C) Andras Varga
// Based on the Mobility Framework's SnrEval by Marc Loebbers
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


//
// Generic radio module. The implementation is largely based on the
// Mobility Framework's SnrEval and Decider modules.
//
// @see Radio
// @author Andras Varga
//
simple GenericRadio
    parameters:
        channelNumber: numeric const, // channel identifier
        transmitterPower : numeric const, // power used for transmission of messages (in mW)
        bitrate: numeric const, // (in bits/s)
        thermalNoise: numeric const, // base noise level (dBm)
        pathLossAlpha: numeric const, // used by the path loss calculation
        snirThreshold: numeric const, // if signal-noise ratio is below this threshold, frame is considered noise (in dB)
        sensitivity: numeric const, // received signals with power below sensitivity are ignored
        headerLengthBits: numeric const, // length of physical layer framing (preamble, etc)
        bandwidth: numeric const, // signal bandwidth, used for bit error calculation
        modulation: string; // "BPSK", "16-QAM", "256-QAM" or "null"; selects bit error calculation method
    gates:
        in: uppergateIn;   // from higher layer protocol (MAC)
        out: uppergateOut; // to decider (decider connects to higher layer protocol, i.e. the MAC)
        in: radioIn; // to receive frames (AirFrame) on the radio channel
endsimple

