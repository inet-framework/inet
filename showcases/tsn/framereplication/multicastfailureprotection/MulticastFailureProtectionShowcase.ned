//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

package inet.showcases.tsn.framereplication.multicastfailureprotection;

import inet.networks.base.TsnNetworkBase;
import inet.node.ethernet.EthernetLink;
import inet.node.tsn.TsnSwitch;

network MulticastFailureProtectionShowcase extends TsnNetworkBase
{
    submodules:
        a: TsnSwitch {
            @display("p=500,200");
        }
        b: TsnSwitch {
            @display("p=700,100");
        }
        d: TsnSwitch {
            @display("p=900,100");
        }
        c: TsnSwitch {
            @display("p=700,300");
        }
        e: TsnSwitch {
            @display("p=900,300");
        }
        f: TsnSwitch {
            @display("p=1100,200");
        }
    connections:
        a.ethg++ <--> EthernetLink <--> c.ethg++;
        a.ethg++ <--> EthernetLink <--> b.ethg++;
        b.ethg++ <--> EthernetLink <--> d.ethg++;
        b.ethg++ <--> EthernetLink <--> e.ethg++;
        c.ethg++ <--> EthernetLink <--> e.ethg++;
        d.ethg++ <--> EthernetLink <--> f.ethg++;
        e.ethg++ <--> EthernetLink <--> f.ethg++;
}
