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

package inet.showcases.tsn.combiningfeatures.invechicle;

import inet.networks.TsnNetworkBase;
import inet.node.ethernet.Eth100M;
import inet.node.inet.IEthernetNetworkNode;
import inet.node.tsn.TsnClock;

network InVehicleNetworkShowcase extends TsnNetworkBase
{
    parameters:
        @display("bgi=background/car;bgb=1280,720");
    types:
        channel Eth1G extends inet.node.ethernet.Eth1G
        {
            @display("ls=,3");
        }
    submodules:
        frontSwitch: <> like IEthernetNetworkNode {
            @display("p=750,350");
        }
        frontLeftSwitch: <> like IEthernetNetworkNode {
            @display("p=900,275");
        }
        frontRightSwitch: <> like IEthernetNetworkNode {
            @display("p=900,417");
        }
        rearSwitch: <> like IEthernetNetworkNode {
            @display("p=475,350");
        }
        rearLeftSwitch: <> like IEthernetNetworkNode {
            @display("p=375,300");
        }
        rearRightSwitch: <> like IEthernetNetworkNode {
            @display("p=375,400");
        }
        obu: <> like IEthernetNetworkNode {
            @display("p=525,275");
        }
        rearDisplay: <> like IEthernetNetworkNode {
            @display("p=525,435");
        }
        hud: <> like IEthernetNetworkNode {
            @display("p=700,275");
        }
        steering: <> like IEthernetNetworkNode {
            @display("p=775,275");
        }
        engineActuator: <> like IEthernetNetworkNode {
            @display("p=750,425");
        }
        lidar: <> like IEthernetNetworkNode {
            @display("p=1000,350");
        }
        rearLeftCamera: <> like IEthernetNetworkNode {
            @display("p=275,250");
        }
        frontLeftCamera: <> like IEthernetNetworkNode {
            @display("p=1000,250");
        }
        rearRightCamera: <> like IEthernetNetworkNode {
            @display("p=275,475");
        }
        frontRightCamera: <> like IEthernetNetworkNode {
            @display("p=1000,475");
        }
        rearLeftWheel: <> like IEthernetNetworkNode {
            @display("p=400,200");
        }
        frontLeftWheel: <> like IEthernetNetworkNode {
            @display("p=850,200");
        }
        rearRightWheel: <> like IEthernetNetworkNode {
            @display("p=400,525");
        }
        frontRightWheel: <> like IEthernetNetworkNode {
            @display("p=850,525");
        }
        masterClock: <default("")> like IEthernetNetworkNode if typename != "" {
            @display("p=625,400;i=device/card");
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
        masterClock.ethg++ <--> Eth100M <--> frontSwitch.ethg++ if exists(masterClock);
}
