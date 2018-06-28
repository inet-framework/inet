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

import inet.applications.httptools.configurator.HttpController;
import inet.node.httptools.HttpDirectHost;


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
        int num_browsers @prompt("Number of browsers") = default(2);
    submodules:
        httpController: HttpController {
            parameters:
                @display("is=s;p=100,100;i=block/cogwheel");
        }
        server1: HttpDirectHost {
            parameters:
                @display("p=100,200;i=device/server");
        }
        server2: HttpDirectHost {
            parameters:
                @display("p=200,200;i=device/server");
        }
        server3: HttpDirectHost {
            parameters:
                @display("p=300,200;i=device/server");
        }
        server4: HttpDirectHost {
            parameters:
                @display("p=400,200;i=device/server");
        }
        server5: HttpDirectHost {
            parameters:
                @display("p=500,200;i=device/server");
        }
        server6: HttpDirectHost {
            parameters:
                @display("p=600,200;i=device/server");
        }
        server7: HttpDirectHost {
            parameters:
                @display("p=700,200;i=device/server");
        }
        server8: HttpDirectHost {
            parameters:
                @display("p=800,200;i=device/server");
        }
        server9: HttpDirectHost {
            parameters:
                @display("p=900,200;i=device/server");
        }
        abc: HttpDirectHost {
            parameters:
                @display("p=500,300;i=device/server");
        }
        client[num_browsers]: HttpDirectHost {
            parameters:
                @display("i=device/laptop;p=300,400,row,400");
        }
    connections allowunconnected:
}

