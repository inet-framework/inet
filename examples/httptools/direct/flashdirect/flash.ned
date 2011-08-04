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

package inet.examples.httptools.direct.flashdirect;

import inet.nodes.httptools.DirectHost;
import inet.world.httptools.HttpController;


//
// Flash crowd simulation 
//
// This is an example of a popularity modification event employed to simulate 
// a flashcrowd. Direct message passing. This scenario is similar to the 10servers
// scenario used to demonstrate the sockets components.
//
network HttpDirectFlash
{
    parameters:
        double num_browsers @prompt("Number of browsers") = default(2);
        @display("bgb=754,559");
    submodules:
        controller: HttpController {
            parameters:
                @display("p=54,60;i=block/cogwheel");
        }
        server1: DirectHost {
            parameters:
                @display("p=68,164;i=device/server");
        }
        server2: DirectHost {
            parameters:
                @display("p=148,164;i=device/server");
        }
        server3: DirectHost {
            parameters:
                @display("p=228,164;i=device/server");
        }
        server4: DirectHost {
            parameters:
                @display("p=308,164;i=device/server");
        }
        server5: DirectHost {
            parameters:
                @display("p=388,164;i=device/server");
        }
        server6: DirectHost {
            parameters:
                @display("p=468,164;i=device/server");
        }
        server7: DirectHost {
            parameters:
                @display("p=548,164;i=device/server");
        }
        server8: DirectHost {
            parameters:
                @display("p=620,164;i=device/server");
        }
        server9: DirectHost {
            parameters:
                @display("p=700,164;i=device/server");
        }
        abc: DirectHost {
            parameters:
                @display("p=365,300;i=device/server_l");
        }
        client[num_browsers]: DirectHost {
            parameters:
                @display("i=device/laptop");
        }
    connections allowunconnected:
}

