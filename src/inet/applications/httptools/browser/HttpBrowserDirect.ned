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

package inet.applications.httptools.browser;

import inet.applications.httptools.common.IHttpDirectApp;


//
// This component is the direct message passing version of the browser simulation
// component. It is derived from HttpBrowserBase. Use this component with the
// DirectHost container module.
//
// Direct message passing is used for transmission, thus bypassing the normal links and
// interfaces. A linkSpeed parameter is hence needed to model transmission delays.
//
// @see DirectHost
// @see HttpBrowserBase
// @see HttpBrowser
//
// @author  Kristjan V. Jonsson
//
simple HttpBrowserDirect like IHttpDirectApp
{
    parameters:
        int httpProtocol = default(11);                // The http protocol: 10 for http/1.0, 11 for http/1.1. Not used at the present time.
        string logFile = default("");                  // Name of a browser log file. Browse events are appended, allowing sharing of file for multiple browsers.
        string scriptFile = default("");               // The browsing script file. Blank to disable.
        double activationTime @unit(s) = default(0s);  // The initial activation delay. Zero to disable (seconds).
        double linkSpeed @unit(bps) = default(11Mbps); // Used to model transmission delays (Bit/s)
        xml config;                                    // The XML configuration file for the browser
        string httpBrowserControllerModule = default("controller"); // the absolute path to the http browser controller, @see HttpController
    gates:
        input httpIn @directIn;
}

