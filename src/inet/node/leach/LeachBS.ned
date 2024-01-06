//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.node.leach;

import inet.node.inet.AdhocHost;
import inet.routing.leach.LeachBS;

module LeachBS extends AdhocHost {
    submodules:
        LeachBS: LeachBS {
            @display("p=825,226");
        }
    connections:
        LeachBS.ipOut --> tn.in++;
        LeachBS.ipIn <-- tn.out++;
        
}
