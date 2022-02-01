//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.showcases.visualizer.canvas.mobility;

import inet.node.inet.AdhocHost;
import inet.visualizer.common.IntegratedVisualizer;

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
