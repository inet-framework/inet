//
// Copyright (C) 2012 Opensim Ltd.
// Author: Tamas Borbely
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

package inet.showcases.general.diffserv;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth10M;
import inet.node.ethernet.EtherSwitch;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import ned.DatarateChannel;

network DiffservNetwork
{
    types:
        channel PppLine extends DatarateChannel
        {
            delay = normal(0.004s, 0.0018s);
            datarate = 128kbps;
        }
    submodules:
        router: Router {
            @display("p=550,150");
        }
        voipPhone1: StandardHost {
            @display("p=250,100;i=misc/voipPhone");
        }
        voipPhone2: StandardHost {
            @display("p=850,100;i=misc/voipPhone");
        }
        client: StandardHost {
            @display("p=250,200");
        }
        server: StandardHost {
            @display("p=850,200");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("is=s;p=100,100");
        }
        etherSwitch: EtherSwitch {
            @display("p=400,150");
        }
        internet: Router {
            @display("p=700,150;i=misc/cloud_l");
        }
    connections:
        voipPhone1.ethg++ <--> Eth10M <--> etherSwitch.ethg++;
        client.ethg++ <--> Eth10M <--> etherSwitch.ethg++;
        etherSwitch.ethg++ <--> Eth10M <--> router.ethg++;
        router.pppg++ <--> PppLine <--> internet.pppg++;
        internet.pppg++ <--> PppLine <--> server.pppg++;
        internet.pppg++ <--> PppLine <--> voipPhone2.pppg++;
}
