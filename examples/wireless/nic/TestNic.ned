//
// Copyright (C) 2006 Andras Varga
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

package inet.examples.wireless.nic;

import inet.base.LifecycleController;
import inet.networklayer.autorouting.ipv4.IPv4NetworkConfigurator;
import inet.nodes.inet.WirelessHost;
import inet.physicallayer.radio.contract.IRadioChannel;


network TestNic
{
    parameters:
        int numHosts;
        string radioChannelType;
    submodules:
        radioChannel: <radioChannelType> like IRadioChannel {
            parameters:
                @display("p=50,50");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                @display("p=50,100");
        }
        lifecycleController: LifecycleController {
            parameters:
                @display("p=50,150");
        }
        host[numHosts]: WirelessHost {
            parameters:
                @display("r=,,#707070;p=250,58");
        }
}
