//
// This program is free Software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You Should have received a copy of the GNU Lesser General Public License
// along with this program.  If Not, See <https://www.gnu.org/licenses/>.
//

package inet.validation.tsn.trafficshaping.asynchronousshaper.icct;

import inet.networks.base.TsnNetworkBase;
import inet.node.ethernet.Eth100M;
import inet.node.tsn.TsnDevice;
import inet.node.tsn.TsnSwitch;

network AsynchronousShaperValidation extends TsnNetworkBase
{
    submodules:
        N1: TsnDevice {
            @display("p=500,150");
        }
        N2: TsnDevice {
            @display("p=300,300");
        }
        N3: TsnDevice {
            @display("p=500,450");
        }
        N4: TsnDevice {
            @display("p=700,150");
        }
        N5: TsnDevice {
            @display("p=700,450");
        }
        N6: TsnDevice {
            @display("p=900,150");
        }
        N7: TsnDevice {
            @display("p=1100,300");
        }
        N8: TsnDevice {
            @display("p=900,450");
        }
        S1: TsnSwitch {
            @display("p=500,300");
        }
        S2: TsnSwitch {
            @display("p=700,300");
        }
        S3: TsnSwitch {
            @display("p=900,300");
        }
    connections:
        N1.ethg++ <--> Eth100M <--> S1.ethg++;
        N2.ethg++ <--> Eth100M <--> S1.ethg++;
        N3.ethg++ <--> Eth100M <--> S1.ethg++;
        N4.ethg++ <--> Eth100M <--> S2.ethg++;
        N5.ethg++ <--> Eth100M <--> S2.ethg++;
        N6.ethg++ <--> Eth100M <--> S3.ethg++;
        N7.ethg++ <--> Eth100M <--> S3.ethg++;
        N8.ethg++ <--> Eth100M <--> S3.ethg++;
        S1.ethg++ <--> Eth100M <--> S2.ethg++;
        S2.ethg++ <--> Eth100M <--> S3.ethg++;
}
