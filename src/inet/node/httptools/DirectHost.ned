//
// Copyright (C) 2009 Kristjan V. Jonsson, LDSS (kristjanvj@gmail.com)
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

package inet.node.httptools;

import inet.applications.httptools.common.IHttpDirectApp;


//
// Simple host for the HttpTools project. The module is derived from the INET
// StandardHost module. All network infrastructure modules were removed;
// the node is a simple container for tcpApps. Suitable apps are the HttpBrowserDirect
// and HttpServerDirect modules from HttpTools.
// See the INET documentation for StandardHost and the provided examples on how
// to assign tcpApps to the module.
//
// @see HttpServerDirect
// @see HttpBrowserDirect
//
// @author Kristjan V. Jonsson (kristjanvj@gmail.com)
//
module DirectHost
{
    parameters:
        @networkNode;
        double numTcpApps = default(0);     // The number of tcpApps in the array
    submodules:
        tcpApp[numTcpApps]: <> like IHttpDirectApp {
            parameters:
                @display("p=163,67;i=block/app");
        }
}


