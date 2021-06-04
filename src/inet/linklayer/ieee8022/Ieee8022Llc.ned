//
// Copyright (C) 2020 OpenSim Ltd.
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

package inet.linklayer.ieee8022;

simple Ieee8022Llc like IIeee8022Llc
{
    parameters:
        @display("i=block/layer");
        @signal[packetDropped](type=inet::Packet);
    gates:
        input upperLayerIn; // TODO accepts packets and socket commands with  @messageKinds(inet::SocketCommandCode), too
        output upperLayerOut; // TODO sends packets and socket indications with @messageKinds(inet::SocketStatusInd), too
        input lowerLayerIn;
        output lowerLayerOut;
}

