//
// Copyright (C) 2009 Kristjan V. Jonsson, LDSS (kristjanvj@gmail.com)
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

