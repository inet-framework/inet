//
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2014 OpenSim Ltd.
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

package inet.networklayer.arp;

//
// TODO
//
moduleinterface IARP
{
    parameters:
        @display("i=block/layer");
        @signal[initiatedARPResolution](type=IARP::Notification);
        @signal[completedARPResolution](type=IARP::Notification);
        @signal[failedARPResolution](type=IARP::Notification);
    gates:
        input netwIn @labels(ARPPacket+Ieee802Ctrl); // incoming ARP requests and replies
        output netwOut @labels(ARPPacket+Ieee802Ctrl);  // outgoing ARP requests/replies, and datagrams with resolved next hop
}

