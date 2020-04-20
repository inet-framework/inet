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

package inet.networks;

import inet.common.packet.recorder.PcapRecorder;
import inet.networklayer.configurator.contract.INetworkConfigurator;
import inet.visualizer.contract.IIntegratedVisualizer;

network NetworkBase
{
    parameters:
        bool recordPcap = default(false);
        int numPcapRecorders = default(recordPcap ? 1 : 0);
        pcapRecorder[*].pcapFile = default("results/" + fullPath() + ".pcap");
    submodules:
        pcapRecorder[numPcapRecorders]: PcapRecorder {
            parameters:
                moduleNamePatterns = ".^";
                @display("p=100,100;is=s");
        }
        visualizer: <default("IntegratedCanvasVisualizer")> like IIntegratedVisualizer if hasVisualizer() {
            parameters:
                @display("p=100,200;is=s");
        }
        configurator: <default("Ipv4NetworkConfigurator")> like INetworkConfigurator if typename != "" {
            parameters:
                @display("p=100,300;is=s");
        }
}
