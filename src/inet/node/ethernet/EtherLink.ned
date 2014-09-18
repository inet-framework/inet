//
// Copyright (C) 2012 Andras Varga
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

package inet.node.ethernet;

import ned.DatarateChannel;

//
// Base for Ethernet link types. Propagation delay can be specified with the
// length of the cable, i.e. in meters instead of nanoseconds or microseconds.
//
channel EtherLink extends DatarateChannel
{
    double length @unit(m) = default(10m);
    delay = replaceUnit(length / 2e8, "s");
}

//
// 10 megabit/sec Ethernet link
//
channel Eth10M extends EtherLink
{
    datarate = 10Mbps;
}

//
// 100 megabit/sec Ethernet link
//
channel Eth100M extends EtherLink
{
    datarate = 100Mbps;
}

//
// 1 gigabit/sec Ethernet link
//
channel Eth1G extends EtherLink
{
    datarate = 1Gbps;
}

//
// 10 gigabit/sec Ethernet link
//
channel Eth10G extends EtherLink
{
    datarate = 10Gbps;
}

//
// 40 gigabit/sec Ethernet link
//
channel Eth40G extends EtherLink
{
    datarate = 40Gbps;
}

//
// 100 gigabit/sec Ethernet link
//
channel Eth100G extends EtherLink
{
    datarate = 100Gbps;
}
