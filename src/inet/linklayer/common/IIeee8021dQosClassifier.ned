//
// Copyright (C) 2015 Andras Varga
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
// along with this program; if not, see http://www.gnu.org/licenses/.
//
// Author: Andras Varga
//

package inet.linklayer.common;

//
// Interface for 802.1d QoS classifiers. For each packet, the classifier
// computes a 802.1d User Priority (UP) value, and sets in on the
// Iee802Ctrl control info before sending out the packet on the "out" gate.
//
moduleinterface IIeee8021dQosClassifier
{
    parameters:
        @display("i=block/fork");
    gates:
        input in;
        output out;
}

