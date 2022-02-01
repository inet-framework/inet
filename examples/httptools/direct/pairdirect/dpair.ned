// ----------------------------------------------------------------------------
//
// HttpTools Project
//
// This file is a part of the HttpTools project. The project was created at
// Reykjavik University, the Laboratory for Dependable Secure Systems (LDSS).
// Its purpose is to create a set of OMNeT++ components to simulate browsing
// behaviour in a high-fidelity manner along with a highly configurable
// Web server component.
//
// Maintainer: Kristjan V. Jonsson (LDSS) kristjanvj@gmail.com
// Project home page: code.google.com/p/omnet-httptools
//
// ----------------------------------------------------------------------------
//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// ----------------------------------------------------------------------------

package inet.examples.httptools.direct.pairdirect;

import inet.applications.httptools.configurator.HttpController;
import inet.node.httptools.HttpDirectHost;


//
// Simple scenario with a pair of nodes. Direct message passing used.
//
network HttpDirectPair
{
    parameters:
    submodules:
        httpController: HttpController {
            parameters:
                @display("is=s;p=100,100;i=block/cogwheel");
        }
        srv: HttpDirectHost {
            parameters:
                @display("p=250,100;i=device/server");
        }
        cli: HttpDirectHost {
            parameters:
                @display("p=400,100;i=device/laptop");
        }
    connections allowunconnected:
}

