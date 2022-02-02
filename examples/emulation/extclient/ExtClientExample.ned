//
// Copyright (C) 2008 Irene Ruengeler
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//

package inet.examples.emulation.extclient;

import inet.node.inet.StandardHost;

network ExtClientExample
{
    submodules:
        peer: StandardHost {
            parameters:
                @display("p=100,100;i=device/laptop");
        }
    connections allowunconnected:
}



