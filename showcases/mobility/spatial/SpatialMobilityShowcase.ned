//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.showcases.mobility.spatial;

import inet.node.inet.StandardHost;
import inet.visualizer.canvas.integrated.IntegratedCanvasVisualizer;


network SpatialMobilityShowcase
{
    parameters:
        int numHosts;
        @display("bgb=1000,1000");
    submodules:
        visualizer: IntegratedCanvasVisualizer {
            @display("p=100,50");
        }
        host[numHosts]: StandardHost {
            @display("i=misc/node_vs");
        }
}
