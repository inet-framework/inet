//
// Copyright (C) 2012 Opensim Ltd.
// Author: Tamas Borbely
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

package inet.networklayer.diffserv;

//
// This module implements a Single Rate Three Color Meter (RFC 2697).
//
// IP packets are classified according to three parameters,
// Committed Information Rate (cir), Committed Burst Size (cbs),
// and Excess Burst Size (ebs), to be either green, yellow or red.
//
// The green traffic is guaranteed to be under (in bits)
// <pre>
//    cir * (t1 - t0) + 8 * cbs
// </pre>
// and green+yellow traffic to be under
// <pre>
//    cir * (t1 - t0) + 8 * (cbs + ebs)
// </pre>
// in every [t0,t1] interval.
//
// When operating in color-aware mode, the meter does not recolor
// a packet that was colored by a previous marker to a lesser
// conforming color.
//
simple SingleRateThreeColorMeter
{
    parameters:
        @display("i=block/timer");
        string interfaceTableModule;   // The path to the InterfaceTable module
        string cir;       // committed information rate, either absolute bitrate (e.g. "100kbps"), or relative to the link's datarate (e.g. "20%")
        int cbs @unit(B); // committed burst size
        int ebs @unit(B); // excess burst size
        bool colorAwareMode = default(false); // enables color-aware mode
    gates:
        input in;
        output greenOut;
        output yellowOut;
        output redOut;
}

