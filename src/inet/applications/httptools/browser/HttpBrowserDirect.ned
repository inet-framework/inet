//
// Copyright (C) 2009 Kristjan V. Jonsson, LDSS (kristjanvj@gmail.com)
//
// SPDX-License-Identifier: GPL-3.0-or-later
//

package inet.applications.httptools.browser;

import inet.applications.httptools.common.IHttpDirectApp;

//
// This component is the direct message passing version of the browser simulation
// component. Use this component with the ~HttpDirectHost container module.
//
// Direct message passing is used for transmission, thus bypassing the normal links and
// interfaces. A linkSpeed parameter is hence needed to model transmission delays.
//
// @see ~HttpDirectHost, ~HttpBrowser
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
        string httpControllerModule = default("httpController"); // the absolute path to the http browser controller, @see HttpController
        @lifecycleSupport;
    gates:
        input httpIn @directIn;
}

