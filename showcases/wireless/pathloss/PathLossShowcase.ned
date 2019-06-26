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
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package inet.showcases.wireless.pathloss;

import inet.environment.common.PhysicalEnvironment;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.AdhocHost;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.integrated.IntegratedVisualizer;


network PathLossShowcase
{
    @display("bgb=1400,400");
    @statistic[receptionPower](source="receptionMinSignalPower(radioMedium.signalArrivalStarted)"; record=last);
    submodules:
        source: AdhocHost {
            @display("p=144.37871,110.05918");
        }
        destination: AdhocHost {
            @display("p=226.62724,110.05918");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=49.704147,35.502964");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=49.704147,85.20711");
        }
        physicalEnvironment: PhysicalEnvironment {
            @display("p=49.704147,135.50298");
        }
}
