//
// Copyright (C) 2011 Philipp Berndt
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

package inet.examples.inet.gnpping;

import inet.world.inet.GnpNetworkConfigurator;
import inet.world.inet.InternetCloud;
import inet.nodes.inet.StandardHost;

network PingNetwork
{
    parameters:
        int numSenders;

    types:
        module GpnStandardHost extends StandardHost {
            string group = default("");   // group ID for delay/jitter etc. parameter group 
        }
    submodules:
        configurator: GnpNetworkConfigurator {
            parameters:
                @display("p=61,163");
        }

        sender[numSenders]: GpnStandardHost;

        recip: GpnStandardHost;

        internet: InternetCloud;

    connections:
        recip.pppg++ <--> internet.pppg++;
        for i=0..numSenders-1 {
            sender[i].pppg++ <--> internet.pppg++;
        }
}
