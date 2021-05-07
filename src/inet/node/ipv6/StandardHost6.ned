//
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
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

package inet.node.ipv6;

import inet.node.inet.StandardHost;

//
// IPv6 host with TCP, SCTP and UDP layers and applications.
// see ~StandardHost for configuration.
//
module StandardHost6 extends StandardHost
{
    parameters:
        @display("i=device/pc2");
        xml routes = default(xml("<routingTable/>"));
        ipv6.routingTable.routes = this.routes;
        hasIpv4 = default(false);
        hasIpv6 = default(true);
        *.routingTableModule = "^.ipv6.routingTable";
}

