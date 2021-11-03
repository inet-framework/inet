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
// along with this program.  If not, see <https://www.gnu.org/licenses/>.//
//

package inet.examples.tsn.gatescheduling;

import inet.networks.base.TsnNetworkBase;
import inet.node.ethernet.EthernetLink;
import inet.node.tsn.TsnDevice;
import inet.node.tsn.TsnSwitch;

module LocalTsnSwitch extends TsnSwitch
{
    @defaultStatistic("gateStateChanged:vector"; module="eth[0].macLayer.queue.gate[0]");
}

network GateSchedulingExample extends TsnNetworkBase
{
    parameters:
        @display("bgi=background/car;bgb=1280,720");
    submodules:
        frontSwitch: LocalTsnSwitch {
            @display("p=742,350");
        }
        frontLeftSwitch: LocalTsnSwitch {
            @display("p=912,276");
        }
        frontRightSwitch: LocalTsnSwitch {
            @display("p=912,417");
        }
        rearSwitch: LocalTsnSwitch {
            @display("p=478,350");
        }
        rearLeftSwitch: LocalTsnSwitch {
            @display("p=381,283");
        }
        rearRightSwitch: LocalTsnSwitch {
            @display("p=381,408");
        }
        hud: TsnDevice {
            @display("p=742,277");
        }
        camera1: TsnDevice {
            @display("p=289,231");
        }
        camera2: TsnDevice {
            @display("p=968,239");
        }
        camera3: TsnDevice {
            @display("p=289,493");
        }
        camera4: TsnDevice {
            @display("p=968,483");
        }
        wheel1: TsnDevice {
            @display("p=400,218");
        }
        wheel2: TsnDevice {
            @display("p=850,218");
        }
        wheel3: TsnDevice {
            @display("p=400,506");
        }
        wheel4: TsnDevice {
            @display("p=850,500");
        }
    connections:
        frontSwitch.ethg++ <--> EthernetLink <--> hud.ethg++;
        frontSwitch.ethg++ <--> EthernetLink <--> rearSwitch.ethg++;
        frontSwitch.ethg++ <--> EthernetLink <--> frontLeftSwitch.ethg++;
        frontSwitch.ethg++ <--> EthernetLink <--> frontRightSwitch.ethg++;
        rearSwitch.ethg++ <--> EthernetLink <--> rearLeftSwitch.ethg++;
        rearSwitch.ethg++ <--> EthernetLink <--> rearRightSwitch.ethg++;
        frontRightSwitch.ethg++ <--> EthernetLink <--> wheel4.ethg++;
        frontRightSwitch.ethg++ <--> EthernetLink <--> camera4.ethg++;
        frontLeftSwitch.ethg++ <--> EthernetLink <--> wheel2.ethg++;
        frontLeftSwitch.ethg++ <--> EthernetLink <--> camera2.ethg++;
        rearLeftSwitch.ethg++ <--> EthernetLink <--> camera1.ethg++;
        rearLeftSwitch.ethg++ <--> EthernetLink <--> wheel1.ethg++;
        rearRightSwitch.ethg++ <--> EthernetLink <--> camera3.ethg++;
        rearRightSwitch.ethg++ <--> EthernetLink <--> wheel3.ethg++;
}
