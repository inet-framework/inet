//
// Copyright (C) 2004 OpenSim Ltd.
// Copyright (C) 2014 OpenSim Ltd.
//
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
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

package inet.networklayer.contract;

moduleinterface IArp
{
    parameters:
        @display("i=block/layer");
        @signal[arpResolutionInitiated](type=inet::IArp::Notification);
        @signal[arpResolutionCompleted](type=inet::IArp::Notification);
        @signal[arpResolutionFailed](type=inet::IArp::Notification);
    gates:
        input ifIn @labels(ArpPacket,Ieee802Ctrl);
        output ifOut @labels(ArpPacket,Ieee802Ctrl);
}

