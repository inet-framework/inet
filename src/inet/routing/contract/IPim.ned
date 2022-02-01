//
// Copyright (C) 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

// Authors: Veronika Rybova, Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)

package inet.routing.contract;

moduleinterface IPim
{
    parameters:
        @display("i=block/network");
    gates:
        input networkLayerIn;
        output networkLayerOut;
}

