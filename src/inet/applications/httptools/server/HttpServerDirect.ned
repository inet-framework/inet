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

package inet.applications.httptools.server;

import inet.applications.httptools.common.IHttpDirectApp;

//
// A Web server module for OMNeT++ simulations and direct message passing between modules.
// This module is intended for use with the HttpDirectHost module and plugs in as a tcpApp.
//
// @see ~HttpServerBase, ~HttpServer, ~HttpDirectHost
//
// @author  Kristjan V. Jonsson
//
simple HttpServerDirect like IHttpDirectApp
{
    parameters:
        string hostName = default("");                      // The domain name of the server.
        int port = default(80);                             // The listening port number.
        int httpProtocol = default(11);                     // The http protocol: 10 for http/1.0, 11 for http/1.1. Not used at the present time.
        string logFile = default("");                       // Name of server log file. Events are appended, allowing sharing of file for multiple servers.
        string siteDefinition = default("");                // The site script file. Blank to disable.
        double activationTime @unit(s) = default(0s);       // The initial activation delay. Zero to disable.
        double linkSpeed @unit(bps) = default(11Mbps);      // Used to model transmission delays.
        xml config;                                         // The XML configuration file for random sites.
        string httpControllerModule = default("httpController"); // Module path for the ~HttpController module.
        @lifecycleSupport;
    gates:
        input httpIn @directIn;
}

