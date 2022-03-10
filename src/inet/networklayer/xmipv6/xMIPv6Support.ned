//
// Copyright (C) 2007
// Faqir Zarrar Yousaf
// Communication Networks Institute, Dortmund University of Technology (TU Dortmund), Germany.
// Christian Bauer
// Institute of Communications and Navigation, German Aerospace Center (DLR)
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//

package inet.networklayer.xmipv6;

import inet.networklayer.contract.IxMIPv6Support;

//
// Implements xMIPv6 (where x = F, H, F-H).
//
module xMIPv6Support like IxMIPv6Support
{
    parameters:
        bool isHomeAgent;
        bool isMobileNode;
        string interfaceTableModule;
        string ipv6TunnelingModule;
        string ipv6NeighbourDiscoveryModule;
        *.interfaceTableModule = default(absPath(this.interfaceTableModule));
        *.ipv6TunnelingModule = default(absPath(this.ipv6TunnelingModule));
        *.ipv6NeighbourDiscoveryModule = default(absPath(this.ipv6NeighbourDiscoveryModule));
        *.bindingUpdateListModule = default(absPath(".buList"));
        *.bindingCacheModule = default(absPath(".bindingCache"));
        @display("i=block/classifier2");
    gates:
        input fromIPv6;
        output toIPv6;
    submodules:
        xMobileIPv6: xMIPv6 {
            isHomeAgent = parent.isHomeAgent;
            isMobileNode = parent.isMobileNode;
            @display("p=250,150");
        }
        buList: BindingUpdateList {
            parameters:
                @display("p=100,100;i=old/harddisk");
        }
        bindingCache: BindingCache {
            parameters:
                @display("p=100,200;i=old/harddisk");
        }
    connections:
        fromIPv6 --> xMobileIPv6.fromIPv6;
        toIPv6 <-- xMobileIPv6.toIPv6;
}

