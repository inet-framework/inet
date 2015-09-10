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
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License version 3
// as published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// ----------------------------------------------------------------------------

package inet.examples.httptools.direct.pairdirect;

import inet.applications.httptools.configurator.HttpController;
import inet.node.httptools.DirectHost;


//
// Simple scenario with a pair of nodes. Direct message passing used.
//
network HttpDirectPair
{
    parameters:
        @display("bgb=474,374");
    submodules:
        controller: HttpController {
            parameters:
                @display("p=62,60;i=block/cogwheel");
        }
        srv: DirectHost {
            parameters:
                @display("p=170,188;i=device/server_l");
        }
        cli: DirectHost {
            parameters:
                @display("p=316,188;i=device/laptop_l");
        }
    connections allowunconnected:
}

