//
// Copyright (C) 2009 Kristjan V. Jonsson, LDSS (kristjanvj@gmail.com)
//
// SPDX-License-Identifier: GPL-3.0-or-later
//

package inet.applications.httptools.common;

//
// Interface for HTTP server and browser models that communicate directly,
// without lower layer protocols. This module is intended for use with the
// ~HttpDirectHost module and plugs in as an app.
//
// @see ~HttpServer, ~HttpDirectHost
//
// @author  Kristjan V. Jonsson
//
moduleinterface IHttpDirectApp
{
    parameters:
        double linkSpeed @unit(bps);  // Used to model transmission delays.
    gates:
        input httpIn @directIn;
}

