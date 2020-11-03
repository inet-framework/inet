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
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
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

