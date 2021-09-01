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

package inet.examples.ethernet.timeawareshaper;

import inet.linklayer.configurator.gatescheduling.contract.IGateSchedulingConfigurator;
import inet.networks.WiredNetworkBase;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.StandardHost;

module Ecu extends StandardHost
{
    parameters:
        @display("i=device/card");
}

module LocalEthernetSwitch extends EthernetSwitch
{
    parameters:
        @defaultStatistic("gateStateChanged:vector"; module="eth[0].macLayer.queue.gate[0]");
}

network TimeAwareShaperExample extends WiredNetworkBase
{
    parameters:
        @display("bgi=background/car;bgb=1280,720");
    submodules:
        gateSchedulingConfigurator: <default("")> like IGateSchedulingConfigurator if typename != "" {
            @display("p=100,400;is=s");
        }
        frontSwitch: LocalEthernetSwitch {
            @display("p=742,350");
        }
        frontLeftSwitch: LocalEthernetSwitch {
            @display("p=912,276");
        }
        frontRightSwitch: LocalEthernetSwitch {
            @display("p=912,417");
        }
        rearSwitch: LocalEthernetSwitch {
            @display("p=478,350");
        }
        rearLeftSwitch: LocalEthernetSwitch {
            @display("p=381,283");
        }
        rearRightSwitch: LocalEthernetSwitch {
            @display("p=381,408");
        }
        hud: Ecu {
            @display("p=742,277");
        }
        camera1: Ecu {
            @display("p=289,231");
        }
        camera2: Ecu {
            @display("p=968,239");
        }
        camera3: Ecu {
            @display("p=289,493");
        }
        camera4: Ecu {
            @display("p=968,483");
        }
        wheel1: Ecu {
            @display("p=400,218");
        }
        wheel2: Ecu {
            @display("p=850,218");
        }
        wheel3: Ecu {
            @display("p=400,506");
        }
        wheel4: Ecu {
            @display("p=850,500");
        }
    connections:
        frontSwitch.ethg++ <--> Eth100M <--> hud.ethg++;
        frontSwitch.ethg++ <--> Eth100M <--> rearSwitch.ethg++;
        frontSwitch.ethg++ <--> Eth100M <--> frontLeftSwitch.ethg++;
        frontSwitch.ethg++ <--> Eth100M <--> frontRightSwitch.ethg++;
        rearSwitch.ethg++ <--> Eth100M <--> rearLeftSwitch.ethg++;
        rearSwitch.ethg++ <--> Eth100M <--> rearRightSwitch.ethg++;
        frontRightSwitch.ethg++ <--> Eth100M <--> wheel4.ethg++;
        frontRightSwitch.ethg++ <--> Eth100M <--> camera4.ethg++;
        frontLeftSwitch.ethg++ <--> Eth100M <--> wheel2.ethg++;
        frontLeftSwitch.ethg++ <--> Eth100M <--> camera2.ethg++;
        rearLeftSwitch.ethg++ <--> Eth100M <--> camera1.ethg++;
        rearLeftSwitch.ethg++ <--> Eth100M <--> wheel1.ethg++;
        rearRightSwitch.ethg++ <--> Eth100M <--> camera3.ethg++;
        rearRightSwitch.ethg++ <--> Eth100M <--> wheel3.ethg++;
}
