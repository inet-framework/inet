//
// This program is property of its copyright holder. All rights reserved.
//

package inet.node.dymo;

import inet.node.inet.AdhocHost;
import inet.routing.dymo.Dymo;

module DymoRouter extends AdhocHost
{
    submodules:
        dymo: Dymo {
            @display("p=700,300");
        }
    connections:
        dymo.ipOut --> tn.in++;
        dymo.ipIn <-- tn.out++;
}
