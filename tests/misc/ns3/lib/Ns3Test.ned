//
// Copyright (C) 2015 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.AdhocHost;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;


network Ns3Test
{
    parameters:
        // double simulationTime;
        int numClients;
        int numApps;
        double serverYPos @unit(m);
        bool withQos;
        double sendInterval @unit(s);
        double appStartTime @unit(s);
        double appStartDelta @unit(s);
        @display("b=297,203");
        **.constraintAreaMinX = 0m;
        **.constraintAreaMinY = 0m;
        **.constraintAreaMinZ = 0m;
        **.constraintAreaMaxX = 20m + numClients*10m;
        **.constraintAreaMaxY = 10m + serverYPos;
        **.constraintAreaMaxZ = 0m;
        **.mobility.initFromDisplayString = false;

    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=61,164");
        }
        srvHost: AdhocHost {
            parameters:
                numUdpApps = numApps;
                mobility.initFromDisplayString = false;
                mobility.initialX = 15m;
                mobility.initialY = serverYPos;
                mobility.initialZ = 0m;
                wlan[*].classifier.typename =  withQos ? "ExampleQosClassifier" : "";
                wlan[*].mac.EDCA = withQos;
                @display("r=,,#707070");
        }
        cliHost[numClients]: AdhocHost {
            parameters:
                numUdpApps = numApps;
                udpApp[*].sendInterval = sendInterval;
                mobility.initialX = 10m + 10m * index();
                mobility.initialY = 10m;
                mobility.initialZ = 0m;
                wlan[*].classifier.typename =  withQos ? "ExampleQosClassifier" : "";
                wlan[*].mac.EDCA = withQos;
                udpApp[*].startTime = appStartTime + appStartDelta * index();
                @display("r=,,#707070");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=61,46");
        }
}

