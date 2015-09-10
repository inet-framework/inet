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

package inet.examples.wireless.crosstalk;

import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.inet.AdhocHost;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211DimensionalRadioMedium;

network Crosstalk
{
    parameters:
        @display("b=297,203");

    submodules:
        configurator: IPv4NetworkConfigurator {
            parameters:
                @display("p=100,50");
        }
        radioMedium: Ieee80211DimensionalRadioMedium {
            parameters:
                @display("p=100,150");
        }
        aNode[2]: AdhocHost {
            parameters:
                @display("r=,,#707070");
        }
        bNode[2]: AdhocHost {
            parameters:
                @display("r=,,#707070");
        }
}
