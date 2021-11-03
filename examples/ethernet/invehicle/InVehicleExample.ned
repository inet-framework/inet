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

package inet.examples.ethernet.invehicle;

import inet.linklayer.configurator.gatescheduling.contract.IGateSchedulingConfigurator;
import inet.networklayer.configurator.contract.INetworkConfigurator;
import inet.networks.base.WiredNetworkBase;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.StandardHost;

module Ecu extends StandardHost
{
    parameters:
        @display("i=device/card");
}

network InVehicleExample extends WiredNetworkBase
{
    parameters:
        @display("bgi=background/car;bgb=1280,720");
    types:
        channel Eth1G extends inet.node.ethernet.Eth1G {
            @display("ls=,3");
        }
    submodules:
        gateSchedulingConfigurator: <default("")> like IGateSchedulingConfigurator if typename != "" {
            @display("p=100,400;is=s");
        }
        streamRedundancyConfigurator: <default("StreamRedundancyConfigurator")> like INetworkConfigurator if typename != "" {
            @display("p=100,500;is=s");
        }
        tsnConfigurator: <default("TsnConfigurator")> like INetworkConfigurator if typename != "" {
            @display("p=100,600;is=s");
        }
        frontSwitch: EthernetSwitch {
            @display("p=742,350");
        }
        frontLeftSwitch: EthernetSwitch {
            @display("p=912,276");
        }
        frontRightSwitch: EthernetSwitch {
            @display("p=912,417");
        }
        rearSwitch: EthernetSwitch {
            @display("p=478,350");
        }
        rearLeftSwitch: EthernetSwitch {
            @display("p=381,283");
        }
        rearRightSwitch: EthernetSwitch {
            @display("p=381,408");
        }
        obu: Ecu {
            @display("p=478,277");
        }
        rearDisplay: Ecu {
            @display("p=478,429");
        }
        hud: Ecu {
            @display("p=703,277");
        }
        steering: Ecu {
            @display("p=781,277");
        }
        engineActuator: Ecu {
            @display("p=742,418");
        }
        lidar: Ecu {
            @display("p=1016,351");
        }
        rearLeftCamera: Ecu {
            @display("p=289,231");
        }
        frontLeftCamera: Ecu {
            @display("p=968,239");
        }
        rearRightCamera: Ecu {
            @display("p=289,493");
        }
        frontRightCamera: Ecu {
            @display("p=968,483");
        }
        rearLeftWheel: Ecu {
            @display("p=400,218");
        }
        frontLeftWheel: Ecu {
            @display("p=850,218");
        }
        rearRightWheel: Ecu {
            @display("p=400,506");
        }
        frontRightWheel: Ecu {
            @display("p=850,500");
        }
    connections:
        frontSwitch.ethg++ <--> Eth1G <--> hud.ethg++;
        frontSwitch.ethg++ <--> Eth1G { @display("m=m,40,40,40,40"); } <--> rearSwitch.ethg++;
        frontSwitch.ethg++ <--> Eth1G { @display("m=m,60,60,60,60"); } <--> rearSwitch.ethg++;
        frontSwitch.ethg++ <--> Eth1G <--> frontLeftSwitch.ethg++;
        frontSwitch.ethg++ <--> Eth1G <--> frontRightSwitch.ethg++;
        frontSwitch.ethg++ <--> Eth100M <--> lidar.ethg++;
        frontSwitch.ethg++ <--> Eth100M <--> steering.ethg++;
        frontSwitch.ethg++ <--> Eth100M <--> engineActuator.ethg++;
        rearSwitch.ethg++ <--> Eth1G <--> rearLeftSwitch.ethg++;
        rearSwitch.ethg++ <--> Eth1G <--> rearRightSwitch.ethg++;
        rearSwitch.ethg++ <--> Eth1G <--> obu.ethg++;
        rearSwitch.ethg++ <--> Eth100M <--> rearDisplay.ethg++;
        frontLeftSwitch.ethg++ <--> Eth1G <--> frontRightSwitch.ethg++;
        frontLeftSwitch.ethg++ <--> Eth100M <--> frontLeftWheel.ethg++;
        frontLeftSwitch.ethg++ <--> Eth100M <--> frontLeftCamera.ethg++;
        frontRightSwitch.ethg++ <--> Eth100M <--> frontRightWheel.ethg++;
        frontRightSwitch.ethg++ <--> Eth100M <--> frontRightCamera.ethg++;
        rearLeftSwitch.ethg++ <--> Eth1G <--> rearRightSwitch.ethg++;
        rearLeftSwitch.ethg++ <--> Eth100M <--> rearLeftCamera.ethg++;
        rearLeftSwitch.ethg++ <--> Eth100M <--> rearLeftWheel.ethg++;
        rearRightSwitch.ethg++ <--> Eth100M <--> rearRightCamera.ethg++;
        rearRightSwitch.ethg++ <--> Eth100M <--> rearRightWheel.ethg++;
}
