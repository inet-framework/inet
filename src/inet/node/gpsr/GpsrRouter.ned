//
// This program is property of its copyright holder. All rights reserved.
//

package inet.node.gpsr;

import inet.node.inet.AdhocHost;
import inet.routing.gpsr.Gpsr;

module GpsrRouter extends AdhocHost
{
    submodules:
        gpsr: Gpsr {
            parameters:
                @display("p=825,226");
        }
    connections:
        gpsr.ipOut --> tn.in++;
        gpsr.ipIn <-- tn.out++;
}

