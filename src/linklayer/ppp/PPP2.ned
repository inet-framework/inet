//
// Copyright (C) 2010 Kyeong Soo (Joseph) Kim
// Copyright (C) 2004 Andras Varga
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



package inet.linklayer.ppp;

//
// \PPP2 implementation.
//
// This is an implementation of PPP without interface registration and
// notification.
//
// @see PPP
//
simple PPP2
{
    parameters:
        int txQueueLimit = default(1000);  // only used if queueModule==""; zero means infinite
        string queueModule = default("");  // name of external (QoS,RED,etc) queue module
        int mtu = default(4470);
        @display("i=block/rxtx");
    gates:
        input netwIn;
        output netwOut;
        inout phys @labels(PPPFrame);
}
