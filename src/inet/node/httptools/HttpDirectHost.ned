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
// Simple host for the HttpTools project. The node is a simple container for apps.
// Suitable apps are the ~HttpBrowserDirect and ~HttpServerDirect modules from HttpTools.
//
// @see ~IHttpDirectApp, ~HttpServerDirect, ~HttpBrowserDirect
//
// @author Kristjan V. Jonsson (kristjanvj@gmail.com)
//
module HttpDirectHost
{
    parameters:
        @networkNode;
        int numApps = default(0);     // The number of apps in the array
    submodules:
        app[numApps]: <> like IHttpDirectApp {
            parameters:
                @display("p=100,100,row,150;i=block/app");
        }
}

