//
// Copyright (C) 2005 Andras Varga
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

package inet.examples.hiddennode_new80211;

import inet.environment.common.PhysicalEnvironment;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.inet.WirelessHost;
import inet.physicallayer.idealradio.IdealRadioMedium;


network HiddenNodeExample
{
    parameters:
        @display("b=297,203;bgb=736.6,557.784");

    submodules:

        hostA: WirelessHost {
            @display("p=300,272");
        }
        hostB: WirelessHost {
            @display("p=450,100");
        }
        hostC: WirelessHost {
            @display("p=600,272");
        }
        radioMedium: IdealRadioMedium {
            @display("p=100,100");
        }
        configurator: IPv4NetworkConfigurator {
            @display("p=100,200");
        }
        physicalEnvironment: PhysicalEnvironment {
            @display("p=100,400");
        }
}
