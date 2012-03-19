//
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


package inet.networklayer.autorouting.ipv4;

//
// TODO
//
// xml:
//   ROOT
//       interface
//          @hosts          "host* router[0..3]"
//          @names          i.e. interface names, like "eth* ppp0"
//          @towards        neighbor host names, like "ap switch"
//          @configure      "true" (default) or "false"  (not implemented!!!)
//          @address        "10.0.x.x"
//          @netmask        "255.255.x.x"
//       route
//          @at             "host* router[0..3]"
//          @host           destination address  (IPvXAddressResolver syntax)
//          @netmask        default: 255.255.255.255; alternative notation: "/23"
//          @gateway        next hop address (IPvXAddressResolver syntax)
//          @interface      output interface name
//          @metric         number
simple IPv4Configurator
{
    parameters:
        @display("i=block/cogwheel_s");
        xml config = default(xml("<empty/>"));
        bool addStaticRoutes = default(true);
        bool optimizeRoutes = default(true);
        bool dumpAddresses = default(false);
        bool dumpRoutes = default(false);
}

