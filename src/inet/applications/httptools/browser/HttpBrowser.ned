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

import inet.applications.contract.ITCPApp;


//
// The component is derived from HttpBrowseBase and is intended to be used
// with the INET frameworks StandardHost and TCP/IP stack modeling.
//
// See the API documentation for HttpBrowserBase and HttpBrowser for further details.
// See the INET documentation for more information on StandardHost and the TCP/IP stack.
//
// The purpose of the HttpBrowser component is to simulate a browser operating on a single host.
// It operates in conjunction with the server component to provide a realistic simulation of Web
// usage patterns.
//
// The activity of the browser is primarily characterized by the expected user behavior, as we are
// modeling an interactive application. Observed browsing patterns are characterized by periods of
// relatively frequent activity, divided by extended pauses. Users browse their favorite news sites,
// read e-mail and research a paper, generating bursts of requests, while periodically returning to
// their daily chores of writing research papers. Activity periods, inter-request delays and request
// size distributions are thus primarily used in our solution to model the browsers behavior.
//
// Browsers send GET requests to server objects, emulating real browsing behavior. The queried
// server returns simulated HTML documents, which can contain references to resources, e.g. images,
// CSS documents and scripts. The browser opens a connection, or number of connections, and
// retrieves each referenced resource. See HTTP messages for further details on the message object.
// The browser does not have any knowledge of the size of the requested object before it is
// returned, as it is solely determined by the server component.
//
// The browser component supports two modes of operation:
//  - Random request mode, in which the browser uses statistical distributions of usage patterns
//       to generate requests to random Web servers.
//  - Scripted mode, in which the browsing behavior is determined by a list of predefined Web
//       sites to visit at specific times.
//
// <h1>Random request mode</h1>
//
// The goal for this mode of operation is for the browser component to exhibit usage patterns,
// which are statistically similar to ones observed in real systems. Parameters are determined
// by statistical distributions, which can be derived from measurements. The parameters supported
// for the browsing simulation are shown in the table below. All are defined in a XML configuration
// file, and assigned to the browser component at run-time using a XML initialization parameter
// and optional XPath-like section specifier. Uniform, normal, exponential, and Zipf random
// distributions are currently supported, in addition to a histogram object for empirical
// distributions.
//
// Parameters:
// - activityPeriod:        The activity period of the browser during a 24h day. We can think of
//                          this as the period which the hosting computer is turned on. A typical
//                          home computer would thus be turned on from about 6 in the afternoon
//                          until midnight, while a office computer would be active from 9 to 5.
// - interSessionInterval:  is the period between sessions, i.e. the time which the computer user
//                          is actively browsing the internet.
// - interRequestInterval:  is the period between page requests during a session, i.e. when the
//                          user enters a URL in his browser or clicks a link.
// - reqInSession:          The number of requests per activity period
// - processingDelay:       The delay in processing of each received HTML document before issuing
//                          requests for referenced resources
// - requestSize:           The size of request, disregarding the TCP/IP headers
//
// The browser selects a server to query in this mode by utilizing the global controller object to
// return a random server reference.
//
// Every browser in a simulation can be configured with different parameters, although this would
// quickly become unwieldy when setting up a large simulation. The flexibility to define groups of
// users, e.g. for light, normal and heavy browsing, is however useful in many cases.
//
// <h1>Scripted mode</h1>
//
// Predefined and repeatable actions by a subset of users in a simulation can be useful in a
// variety of situations. One example is an experiment in which we let the majority of browsers
// peruse HTML pages according to the nominal usage pattern, while a small subset behaves in a
// known manner, e.g. issuing anomalous requests consistent with a XSS attack. The general
// population here provides "background noise", behind which the attack may be hard to detect.
// Methods for detection of such events are of particular interest in our current research, which
// provided the inspiration for the creation of HttpTools.
//
// We thus support a scripted mode for our browser object, in which traces are replayed to inject
// browse events at specific times. We can of course mix browser nodes employing the random and
// scripted modes in a scenario. %One application would be to inject traces collected on a real
// browser into a scenario, with a number of simulated browsers providing background traffic.
//
//The script file is specified by setting a scriptFile initialization parameter. The file format is:
//
// <pre>{simulation time};{resource URL} </pre>
//
// The simulation time parameter indicates the time at which the request for a specific resource,
// indicated by the resource URL, is made.
//
// @author  Kristjan V. Jonsson
//
simple HttpBrowser like ITCPApp
{
    parameters:
        int httpProtocol = default(11);                 // The http protocol: 10 for http/1.0, 11 for http/1.1. Not used at the present time.
        string logFile = default("");                   // Name of a browser log file. Browse events are appended, allowing sharing of file for multiple browsers.
        string scriptFile = default("");                // The browsing script file. Blank to disable.
        double activationTime @unit("s") = default(0s); // The initial activation delay. Zero to disable. This is the time at which the browser first comes to life in a simulation scenario. Note that this can be any random distribution which OMNeT++ supports.
        xml config;                                     // The XML config file
        string httpBrowserControllerModule = default("controller"); // the absolute path to the http browser controller, @see HttpController
    gates:
        input tcpIn;
        output tcpOut;
}

