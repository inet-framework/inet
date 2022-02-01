//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.showcases.visualizer.osg.networknode;
import inet.node.inet.AdhocHost;
import inet.visualizer.common.IntegratedVisualizer;


network NetworkNodeVisualizerShowcase
{
    @display("bgb=300,300");
    submodules:
        visualizer: IntegratedVisualizer {
            @display("p=50,50");
        }
        car: AdhocHost {
            @display("p=168,117;i=misc/car2");
        }
        pedestrian: AdhocHost {
            @display("p=113,156;i=misc/person");
        }
}

