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

package inet.examples.internetcloud;

import inet.networklayer.internetcloud.InternetCloudNetworkConfigurator;
import inet.nodes.inet.StandardHost;
import inet.nodes.internetcloud.InternetCloud;
import ned.DatarateChannel;


network PingNetwork
{
    parameters:
        int numSenders;

    types:
        module GnpStandardHost extends StandardHost
        {
            string group = default("");   // group ID for delay/jitter etc. parameter group 
        }
        channel C extends DatarateChannel
        {
            delay = 0;
            datarate = 100Mbps;
        }
    submodules:
        configurator: InternetCloudNetworkConfigurator {
            parameters:
                @display("p=61,163");
        }

        sender[numSenders]: GnpStandardHost;
        recip: GnpStandardHost;
        internet: InternetCloud;
    connections:
        recip.pppg++ <--> C <--> internet.pppg++;
        for i=0..numSenders-1 {
            sender[i].pppg++ <--> C <--> internet.pppg++;
        }
}

