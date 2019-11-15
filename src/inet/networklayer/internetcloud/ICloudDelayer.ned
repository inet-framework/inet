//
// Copyright (C) 2012 OpenSim Ltd.
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
// @author Zoltan Bojthe
//

package inet.networklayer.internetcloud;

//
// Delay module interface for InternetCloud.
//
// Delays or drops the packets based on which interface card the packet
// has arrived on and on which interface it is leaving the cloud.
// This module registers self to ~Ipv4 module as an INetfilter::IHook hook
// and uses the datagramForwardHook for delay/drop packets
//
// @see ~InternetCloud
//
moduleinterface ICloudDelayer
{
    parameters:
        string networkProtocolModule;
        @display("i=block/delay");
}

