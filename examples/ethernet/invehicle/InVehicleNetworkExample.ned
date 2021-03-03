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

import inet.linklayer.configurator.StreamReservationConfigurator;
import inet.networks.WiredNetworkBase;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.StandardHost;
import inet.queueing.contract.IPacketClassifier;
import inet.queueing.contract.IPacketGate;
import inet.queueing.contract.IPacketQueue;
import inet.queueing.contract.IPacketScheduler;
import inet.queueing.queue.CompoundPacketQueueBase;

module Ecu extends StandardHost
{
    parameters:
        @display("i=device/card");
}

module InVehicleNetworkExampleQueue extends CompoundPacketQueueBase
{
    parameters:
        int numQueues;
        double bitrate @unit(bps);
        *.bitrate = default(bitrate);
        *.guardBand = default(replaceUnit(96.0 / dropUnit(bitrate), "s"));
    submodules:
        classifier: <default("ContentBasedClassifier")> like IPacketClassifier {
            parameters:
                @display("p=100,100");
        }
        queue[numQueues]: <default("PacketQueue")> like IPacketQueue {
            parameters:
                @display("p=300,100,column,125");
        }
        gate[numQueues]: <default("PeriodicGate")> like IPacketGate {
            parameters:
                @display("p=500,100,column,125");
        }
        scheduler: <default("PriorityScheduler")> like IPacketScheduler {
            parameters:
                @display("p=700,100");
        }
    connections:
        in --> { @display("m=w"); } --> classifier.in;
        for i=0..sizeof(queue)-1 {
            classifier.out++ --> queue[i].in;
            queue[i].out --> gate[i].in;
            gate[i].out --> scheduler.in++;
        }
        scheduler.out --> { @display("m=e"); } --> out;
}

network InVehicleNetworkExample extends WiredNetworkBase
{
    parameters:
        bool hasStreamReservationConfigurator = default(false);
        @display("bgi=background/car;bgb=1280,720");
    types:
        channel Eth1G extends inet.node.ethernet.Eth1G
        {
            @display("ls=,3");
        }
    submodules:
        streamReservationConfigurator: StreamReservationConfigurator if hasStreamReservationConfigurator {
            @display("p=100,400;is=s");
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
            @display("p=263,351");
        }
        hud: Ecu {
            @display("p=742,429");
        }
        steering: Ecu {
            @display("p=742,277");
        }
        lidar: Ecu {
            @display("p=1016,351");
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
        frontSwitch.ethg++ <--> Eth1G <--> hud.ethg++;
        frontSwitch.ethg++ <--> Eth1G { @display("m=m,40,40,40,40"); } <--> rearSwitch.ethg++;
        frontSwitch.ethg++ <--> Eth1G { @display("m=m,60,60,60,60"); } <--> rearSwitch.ethg++;
        frontSwitch.ethg++ <--> Eth100M <--> lidar.ethg++;
        frontSwitch.ethg++ <--> Eth100M <--> frontLeftSwitch.ethg++;
        frontSwitch.ethg++ <--> Eth100M <--> frontRightSwitch.ethg++;
        frontSwitch.ethg++ <--> Eth100M <--> steering.ethg++;
        rearSwitch.ethg++ <--> Eth100M <--> rearLeftSwitch.ethg++;
        rearSwitch.ethg++ <--> Eth100M <--> rearRightSwitch.ethg++;
        rearSwitch.ethg++ <--> Eth100M <--> obu.ethg++;
        frontRightSwitch.ethg++ <--> Eth100M <--> wheel4.ethg++;
        frontRightSwitch.ethg++ <--> Eth100M <--> camera4.ethg++;
        frontLeftSwitch.ethg++ <--> Eth100M <--> wheel2.ethg++;
        frontLeftSwitch.ethg++ <--> Eth100M <--> camera2.ethg++;
        rearLeftSwitch.ethg++ <--> Eth100M <--> camera1.ethg++;
        rearLeftSwitch.ethg++ <--> Eth100M <--> wheel1.ethg++;
        rearRightSwitch.ethg++ <--> Eth100M <--> camera3.ethg++;
        rearRightSwitch.ethg++ <--> Eth100M <--> wheel3.ethg++;
}
