//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.node.leach;

import inet.node.inet.AdhocHost;
import inet.routing.leach.Leach;

module LeachNode extends AdhocHost {
    submodules:
        LeachNode: Leach {
            @display("p=825,226");
        }
    connections:
        LeachNode.ipOut --> tn.in++;
        LeachNode.ipIn <-- tn.out++;
        
}