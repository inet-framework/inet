//
// Copyright (C) 2008 Irene Ruengeler
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
//

package inet.examples.emulation.pinger;

import inet.node.inet.StandardHost;

network ExtPingerExample
{
    submodules:
        peer: StandardHost {
            parameters:
                @display("p=100,100;i=device/laptop");
        }
    connections allowunconnected:
}

