//
// Copyright (C) 2009 Kristjan V. Jonsson, LDSS (kristjanvj@gmail.com)
//
// SPDX-License-Identifier: GPL-3.0-or-later
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

