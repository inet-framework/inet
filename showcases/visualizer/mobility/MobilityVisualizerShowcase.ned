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

package inet.showcases.visualizer.mobility;

import inet.node.inet.AdhocHost;
import inet.visualizer.integrated.IntegratedVisualizer;

network MobilityVisualizerShowcase
{
    @display("bgb=800,600");
    submodules:
        visualizer: IntegratedVisualizer {
            parameters:
                @display("p=100,50");
        }
        host1: AdhocHost {
            @display("p=538,158");
        }
        host2: AdhocHost {
            @display("p=573.8,384.14398");
        }
        host3: AdhocHost {
            @display("p=345.488,271.8");
        }
}
