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

package inet.showcases.visualizer.linkbreak;

import inet.common.scenario.ScenarioManager;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.AdhocHost;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.integrated.IntegratedVisualizer;


network LinkBreakA
{
    @display("bgb=865,515");
    submodules:
        source: AdhocHost {
            @display("p=240,285");
        }
        adhocHost1: AdhocHost {
            @display("p=395,261");
        }
        adhocHost2: AdhocHost {
            @display("p=497,134");
        }
        destination1: AdhocHost {
            @display("p=697,119");
        }
        destination2: AdhocHost {
            @display("p=509,378");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=71,44");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=70,105");
        }
        visualizer: IntegratedVisualizer {
            @display("p=71,163");
        }
        scenarioManager: ScenarioManager {
            @display("p=70.60625,294.37375");
        }
}
