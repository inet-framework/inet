//
// Copyright (C) 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

// Authors: Veronika Rybova, Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)

package inet.routing.contract;

//
// Defines a module interface for Protocol Independent Multicast (PIM) routing
// protocol implementations. PIM is used for multicast routing across IP networks,
// building distribution trees for multicast data.
//
moduleinterface IPim
{
    parameters:
        @display("i=block/network");
    gates:
        input networkLayerIn;
        output networkLayerOut;
}
